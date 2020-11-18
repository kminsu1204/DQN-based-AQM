/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Minsu Kim
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Minsu Kim <minsu1.kim@ryerson.ca>
 */


#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/drop-tail-queue.h"
#include "dqn-queue-disc.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DqnQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (DqnQueueDisc);

TypeId DqnQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DqnQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<DqnQueueDisc> ()
    .AddAttribute ("UpdatePeriod",
                   "Period for selecting an action",
                   TimeValue (Seconds (0.001)),
                   MakeTimeAccessor (&DqnQueueDisc::m_updatePeriod),
                   MakeTimeChecker ())
		.AddAttribute ("MaxSize",
                   "The maximum number of packets accepted by this queue disc",
                   QueueSizeValue (QueueSize ("50p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
		.AddAttribute ("DequeueThreshold",
                   "Minimum queue size in byte before dequeue rate is measured",
                   UintegerValue (2000),
                   MakeUintegerAccessor (&DqnQueueDisc::m_dequeueThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DesiredQueueDelay",
                   "Desired queueing delay",
                   TimeValue (Seconds (0.03)),
                   MakeTimeAccessor (&DqnQueueDisc::m_desiredQueueDelay),
                   MakeTimeChecker ())
		.AddAttribute ("PhysicalDataRate",
                   "Physical data rate of the device",
                   DataRateValue (DataRate ("6.5Mbps")),
                   MakeDataRateAccessor (&DqnQueueDisc::m_physicalDataRate),
                   MakeDataRateChecker ())
		.AddAttribute ("BaselineRTT",
                   "Round trip time on p2p link bewteen edge device and cloud gateway",
                   UintegerValue (0),
                   MakeUintegerAccessor (&DqnQueueDisc::m_rtt),
                   MakeUintegerChecker<uint32_t> ())
		.AddAttribute ("IotGroup",
                   "Number of IoT group",
                   UintegerValue (1),
                   MakeUintegerAccessor (&DqnQueueDisc::m_nIotGroup),
                   MakeUintegerChecker<uint32_t> ())
		.AddAttribute ("RewardsWeight",
                   "Weight for trade-off between delay and drop-rate",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&DqnQueueDisc::m_rewardsWeight),
                   MakeDoubleChecker <double> ())
		.AddAttribute ("Episode",
                   "N th episode",
                   UintegerValue (1),
                   MakeUintegerAccessor (&DqnQueueDisc::m_episode),
                   MakeUintegerChecker<uint32_t> ())
		.AddAttribute ("TrainingMode",
                   "True if it is training mode / False (testing mode)",
                   BooleanValue (true),
                   MakeBooleanAccessor (&DqnQueueDisc::m_trainingMode),
                   MakeBooleanChecker ())
		.AddAttribute ("StatusTrigger",
                   "Show status in std output",
                   BooleanValue (false),
                   MakeBooleanAccessor (&DqnQueueDisc::m_statusTrigger),
                   MakeBooleanChecker ())
  ;

  return tid;
}

DqnQueueDisc::DqnQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE)
{
  NS_LOG_FUNCTION (this);
	m_eventId = Simulator::Schedule (Seconds (0.0), &DqnQueueDisc::SelectAction, this);
}

DqnQueueDisc::~DqnQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
DqnQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

	std::cout << std::endl << "Sum of rewards: " << m_rewardsSum << std::endl;
	std::cout << "Episode " << m_episode << " step count: " << m_episodeStepCount << std::endl;
	std::cout << "Number of drop action: " << m_dropCount << ", non-drop action: " << m_nonDropCount << ", random action: " << m_randomCount << std::endl << std::endl;

	Simulator::Remove (m_eventId);
  QueueDisc::DoDispose ();
}


void
DqnQueueDisc::InitializeParams (void)
{
	m_gamma = 0.99;

	/*m_enqueueRate = 0.0;
	m_enqueueMeasurement = false;
	m_enqueueStart = 0;
	m_enqueueCount = COUNT_INVALID;*/

	m_dequeueRate = 0.0;
	m_dequeueMeasurement = false;
	m_dequeueStart = 0;
	m_dequeueCount = COUNT_INVALID;

	m_actionTrigger = true;
	m_currQueueDelay = Seconds(0);
	m_oldQueueDelay = Seconds(0);
	m_enqueuedPacket = 0;
	m_droppedPacket = 0;
	m_rewardsSum = 0;
	m_done = false;
	m_batchSize = 32;
	m_targetUpdatePeriod = 10;
	m_episodeStepCount = 0;
	m_action = 1;

	m_dropCount = 0;
	m_nonDropCount = 0;
	m_randomCount = 0;

	ConstructApproximationFunction();	// Construct MLPs in initialization step
}

void DqnQueueDisc::SelectAction(void) {
	// Work when dequeue rate is larget than 0
	if (m_dequeueRate > 0) {
		if (m_statusTrigger == true) {
			double now = Simulator::Now ().GetSeconds ();
			std::cout << std::endl << "Current virtual time: " << now << std::endl;
			std::cout << "*** Current State ***" << std::endl;
		}
		m_currState.clear();
		m_currState = GetObservation();

		// predict and take the action using current state	
		float explore = tiny_dnn::uniform_rand(0.0f, 1.0f);
		// give random action when only it is training mode
		if (m_trainingMode == true && explore < 1.0 / (((double)m_episode/5) + 1)) {
			float randomAction = tiny_dnn::uniform_rand(0.0f, 1.0f);
			
			if (randomAction < 0.5) {
				m_action = 0;	// drop state
				if (m_statusTrigger == true)
					std::cout << "Set to drop state randomly" << std::endl;
		
				DropByDQN();
			}
			else {
				m_action = 1;	// non-drop state	
				if (m_statusTrigger == true)
					std::cout << "Set to non-drop state randomly" << std::endl;
			
			}
			m_randomCount++;
		}
		else {
			tiny_dnn::vec_t prediction = m_mainEstimator.predict(m_currState);
			if (prediction[0] > prediction[1]) {
				m_action = 0;

				if (m_statusTrigger == true)
					std::cout << "Set to drop state by DQN" << std::endl;
				
				m_dropCount++;
				DropByDQN();
			}
			else {
				m_action = 1;		
				if (m_statusTrigger == true)
					std::cout << "Set to non-drop state by DQN" << std::endl;
				
				m_nonDropCount++;
			}
			
			if (m_statusTrigger == true) {
				std::cout << "prediction[0]: " << prediction[0] << std::endl;
				std::cout << "prediction[1]: " << prediction[1] << std::endl;
			}
		}

		m_actionTrigger = false;	// Set trigger false before going to next state (to avoid inf. loop in "select action" function)
		m_enqueuedPacket = 0;	// Initialize N of enqueued / dropped packets before going to next state
		m_droppedPacket = 0;
		m_oldQueueDelay = m_currQueueDelay;
		m_eventId = Simulator::Schedule (m_updatePeriod, &DqnQueueDisc::CalculateRewards, this);

	}

	if (m_actionTrigger == true) {	// Keep checking if queue delay is 0
		m_eventId = Simulator::Schedule (m_updatePeriod, &DqnQueueDisc::SelectAction, this);
	}
}


void DqnQueueDisc::DropByDQN(void) {
	if (!GetInternalQueue(0)->IsEmpty()) {
		Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();
		
		// Drops due to the DQN action
		DropAfterDequeue (item, DQN_PROB_DROP);
		PacketProcessingRate(item, m_dequeueMeasurement, m_dequeueThreshold, m_dequeueStart, m_dequeueCount, m_dequeueRate);
	}
}

void DqnQueueDisc::CalculateRewards(void) {
	if (m_statusTrigger == true)
		std::cout << std::endl <<  "*** Next State ***" << std::endl;
	
	m_nextState.clear();
	m_nextState = GetObservation();

	if (m_currState != m_nextState) {
		if (m_statusTrigger == true)
			std::cout << std::endl << "*** Rewards ***" << std::endl;
		
		double delayReward;
		double enqueueReward;

		double enqueueRate;
		if (m_enqueuedPacket + m_droppedPacket == 0)	// calculate enqueue rate for reward function
			enqueueRate = 0;
		else
			enqueueRate = (double)m_enqueuedPacket / (m_enqueuedPacket + m_droppedPacket);
		
		double phyDataRate = (double)m_physicalDataRate.GetBitRate() / 8.0;	// Byte
		double minDelay = ((double)GetInternalQueue (0)->GetNBytes() / phyDataRate);
		
		
		// Calculate delay reward and enqueue reward
		delayReward = m_rewardsWeight * (m_desiredQueueDelay.GetSeconds() - m_currQueueDelay.GetSeconds());
		enqueueReward = (1 - m_rewardsWeight) * (minDelay - m_desiredQueueDelay.GetSeconds()) * enqueueRate;

		if (m_action == 1 && minDelay < m_desiredQueueDelay.GetSeconds())
			enqueueReward = 0;	// When enqueue reward is minus in non-drop state 
	 

		if (m_currState[0] == 0 && m_action == 0) {	// To avoid infinite-drop state
			m_singleReward = -1;
			//m_done = true;
		}
		else if ( GetCurrentSize().GetValue() == 0 && m_action == 1 && enqueueRate == 0) {
			m_singleReward = 0;
		}
		//else if (m_action == 1 && GetCurrentSize.GetValue() == GetMaxSize().GetValue() && m_droppedPacket > 0)
		/*else if (m_action == 1 && m_oldQueueDelay < m_currQueueDelay) {
			m_singleReward = -1;
		}
		else if (m_oldQueueDelay == m_currQueueDelay) {
			m_singleReward = 0;
		}*/
		else {
			m_singleReward = (delayReward + enqueueReward);	// Gain for reward
		}

		m_singleReward = std::max(-1.0, m_singleReward);	// Clipped by min / max value
		m_singleReward = std::min(1.0, m_singleReward);

		
		if (m_statusTrigger == true) {
			std::cout << "enqueue rate: " << enqueueRate << std::endl;
			std::cout << "minimum delay: " << minDelay << "s" << std::endl;
			std::cout << "delay reward: " << delayReward << std::endl;
			std::cout << "enqueue reward: " << enqueueReward << std::endl;
			std::cout << "reward: " << m_singleReward << std::endl << std::endl;;
		}		

		m_rewardsSum += m_singleReward;

		m_episodeStepCount++;	// Increment of step count
		
		if (m_trainingMode == true) {
			experience_t experience{m_currState, m_action, m_singleReward, m_nextState, m_done};
			m_replayMemory.push_back(experience);

			// When episode is done (failed) before first training
			if (m_replayMemory.size() < m_batchSize && m_done == true) {
				// Shuffle and put all experiences into UpdateModel
				std::random_shuffle(m_replayMemory.begin(), m_replayMemory.end());
				m_batchSize = m_replayMemory.size();
				UpdateModel(m_replayMemory);
			}

			// Update Model once the size of replay memory is greater than mini batch size
			if (m_replayMemory.size() >= m_batchSize) {
				memory_t minibatch;

				std::random_shuffle(m_replayMemory.begin(), m_replayMemory.end());
				for (uint32_t i = 0; i < m_batchSize; i++) {
					minibatch.push_back(m_replayMemory[i]);
				}

				UpdateModel(minibatch);
			}
		}


		if ( (m_currQueueDelay.GetSeconds () < 0.5 * m_desiredQueueDelay.GetSeconds ()) && (m_oldQueueDelay.GetSeconds () < (0.5 * m_desiredQueueDelay.GetSeconds ())) && (m_action == 1) && (m_dequeueRate > 0)) {
			m_dequeueCount = COUNT_INVALID;
			m_dequeueRate = 0.0;
			////std::cout << "Dequeue variables are initialized" << std::endl;
		}
	}

	else
		if (m_statusTrigger == true)
			std::cout << std::endl<< "Next state is same as current state.." << std::endl;
	
	m_action = 1;
	m_actionTrigger = true;

	m_eventId = Simulator::Schedule (NanoSeconds(0), &DqnQueueDisc::SelectAction, this);
}


void DqnQueueDisc::UpdateModel(memory_t m) {
	std::vector<observation_t> currStates;
	int done;
	std::vector<tiny_dnn::vec_t> targets;
	
	for(uint32_t i = 0; i < m.size(); i++) {
		currStates.push_back(std::get<0>(m[i]));	// m[i][0] = current state
		
		if (std::get<4>(m[i]) == true)	// m[i][4] = done (true or false)
			done = 0;
		else
			done = 1;

		tiny_dnn::vec_t prediction = m_targetEstimator.predict(std::get<3>(m[i]));	// m[i][3] = next state
		double max = *std::max_element(prediction.begin(), prediction.end());	// Get greatest prediction probability	
		
		// Compute TD error
		double q_target = std::get<2>(m[i]) + (m_gamma * max * done);	// m[i][2] = reward
		
		prediction.clear();
		prediction = m_mainEstimator.predict(currStates[i]);
		////std::cout << "prediction[0] : " << prediction[0] << " , [1]: " << prediction[1] << std::endl;
		prediction[std::get<1>(m[i])] = q_target;	// m[i][1] = action
		////std::cout << "prediction[0] : " << prediction[0] << " , [1]: " << prediction[1] << std::endl;
		targets.push_back(prediction);
	}

	tiny_dnn::adam optimizer;	// default learning rate 0.001
	//tiny_dnn::RMSprop optimizer;	// default lr 0.0001
	optimizer.alpha = 0.01;

	// Train the network (mini-batch size, 1 epoch)
	if (m_statusTrigger == true)
		std::cout << "Training..." << std::endl;
	
	m_mainEstimator.fit<tiny_dnn::mse>(optimizer, currStates, targets, m_batchSize, 1);
	
	// Save the model
	m_mainEstimator.save("dqnMainEstimator" + std::to_string((int)(m_physicalDataRate.GetBitRate()/1e+6)) + std::to_string(m_nIotGroup) + std::to_string((int)(m_rewardsWeight * 10)) + std::to_string(m_rtt));
	m_targetEstimator.save("dqnTargetEstimator" + std::to_string((int)(m_physicalDataRate.GetBitRate()/1e+6)) + std::to_string(m_nIotGroup) + std::to_string((int)(m_rewardsWeight * 10)) + std::to_string(m_rtt));
	
	// Update target estimator using main estimator in target update period
	if (m_episodeStepCount % m_targetUpdatePeriod == 0)
		m_targetEstimator.load("dqnMainEstimator" + std::to_string((int)(m_physicalDataRate.GetBitRate()/1e+6)) + std::to_string(m_nIotGroup) + std::to_string((int)(m_rewardsWeight * 10)) + std::to_string(m_rtt));
	
	// If simulation is failed, update target estimator using main estimator and force to terminate the program (the program is restarted in bash script)
	if (m_done == true) {
	m_targetEstimator.load("dqnMainEstimator" + std::to_string((int)(m_physicalDataRate.GetBitRate()/1e+6)) + std::to_string(m_nIotGroup) + std::to_string((int)(m_rewardsWeight * 10)) + std::to_string(m_rtt));
	//std::cout << std::endl << "Sum of rewards: " << m_rewardsSum << std::endl;
	//std::cout << "Episode " << m_episode << " step count: " << m_episodeStepCount << std::endl << std::endl;
		exit(1);
	}
}


void DqnQueueDisc::ConstructApproximationFunction(void) {
	/*
	3 inputs

	1. current queue length in packet
	2. dequeue rate
	3. current queuing delay
	*/
	if (m_episode == 1 && m_trainingMode == true) {	// If this is the first episode, construct MLPs
		m_mainEstimator << tiny_dnn::layers::fc(3, 64) << tiny_dnn::activation::relu() // [0, inf)
			 << tiny_dnn::layers::fc(64, 64) << tiny_dnn::activation::relu()
			 << tiny_dnn::layers::fc(64, 2) << tiny_dnn::activation::softmax(); // (0~1)	 
		
		m_targetEstimator << tiny_dnn::layers::fc(3, 64) << tiny_dnn::activation::relu() // [0, inf)
			 << tiny_dnn::layers::fc(64, 64) << tiny_dnn::activation::relu()
			 << tiny_dnn::layers::fc(64, 2) << tiny_dnn::activation::softmax(); // (0~1)	 
		
		m_mainEstimator.weight_init(tiny_dnn::weight_init::xavier());	// Xavier initializer
		m_targetEstimator.weight_init(tiny_dnn::weight_init::xavier());
	}

	else {	// Load the saved MLPs
		m_mainEstimator.load("dqnMainEstimator" + std::to_string((int)(m_physicalDataRate.GetBitRate()/1e+6)) + std::to_string(m_nIotGroup) + std::to_string((int)(m_rewardsWeight * 10)) + std::to_string(m_rtt));
		m_targetEstimator.load("dqnTargetEstimator" + std::to_string((int)(m_physicalDataRate.GetBitRate()/1e+6)) + std::to_string(m_nIotGroup) + std::to_string((int)(m_rewardsWeight * 10)) + std::to_string(m_rtt));
	}
	/*
 	outputs
	1. drop probability
	2. non-drop probability
	*/
}

observation_t DqnQueueDisc::GetObservation(void) {
	// Get state (current queue length, dequeue rate, gap bewteen two dequeuing)
	observation_t ob;

	// Calculate current queue delay for get observation
	if (m_dequeueRate > 0) {
		m_currQueueDelay = Time (Seconds (GetInternalQueue (0)->GetNBytes () / m_dequeueRate));
	}
	else {
		m_currQueueDelay = Time (Seconds(0));
	}

	ob.push_back(GetCurrentSize ().GetValue());
	ob.push_back(m_dequeueRate * 8 / 1e+6);	// Convert to Mbps
	ob.push_back(m_currQueueDelay.GetSeconds());
	
	if (m_statusTrigger == true) {
	////std::cout << "Current queue size in bytes: " << GetInternalQueue (0)->GetNBytes() << "bytes" << std::endl;
		std::cout << "Current queue size in packet: " << GetCurrentSize ().GetValue() << "p" << std::endl;
		std::cout << "dequeue rate: " << m_dequeueRate * 8 / 1e+6 << "Mbps" << std::endl;
		std::cout << "Current queue delay: " << m_currQueueDelay.GetSeconds() << "s" << std::endl;
	}

	return ob;
}


void DqnQueueDisc::PacketProcessingRate(Ptr<QueueDiscItem>& item, bool& measurement, uint32_t& threshold, double& start, uint64_t& count, double& rate) {	
	// Measure en/dequeue rate after processing the item
	double now = Simulator::Now ().GetSeconds ();
	uint32_t pktSize = item->GetSize();

	if ((GetInternalQueue (0)->GetNBytes () >= threshold) && (!measurement)) {
		start = now;
		count = 0;
		measurement = true;
	}
	if (measurement) {
		count += pktSize;

		if (count >= threshold) {
			double tmp = now - start;

			if (tmp > 0) {
				if (rate == 0) {
					rate = (double)count / tmp;
				}
				else {	// Proportion of old/new processing rate can be changed
					rate = (0.5 * rate) + (0.5 * (count / tmp));
				}
			}
			// Restart a measurement cycle if number of packets in queue exceeds the threshold
			if (GetInternalQueue (0)->GetNBytes () > threshold) {
				start = now;
				count = 0;
				measurement = true;
			}
			else {
				count = 0;
				measurement = false;
			}
		}
	}
}

bool
DqnQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

	QueueSize nQueued = GetCurrentSize ();

	//PacketProcessingRate(item, m_enqueueMeasurement, m_enqueueThreshold, m_enqueueStart, m_enqueueCount, m_enqueueRate);

  if (nQueued + item > GetMaxSize ())
    {
			m_droppedPacket++;
      // Drops due to limitted queue size
      DropBeforeEnqueue (item, EXCEEDED_DROP);
      return false;
    }

	m_enqueuedPacket++;
  
	// No drop
  bool retval = GetInternalQueue (0)->Enqueue (item);

	////std::cout << "Packet comes in! Length of Queue: "<< GetCurrentSize () << std::endl;

  // If Queue::Enqueue fails, QueueDisc::DropBeforeEnqueue is called by the
  // internal queue because QueueDisc::AddInternalQueue sets the trace callback

  NS_LOG_LOGIC ("\t bytesInQueue  " << GetInternalQueue (0)->GetNBytes ());
  NS_LOG_LOGIC ("\t packetsInQueue  " << GetInternalQueue (0)->GetNPackets ());

  return retval;
}

Ptr<QueueDiscItem>
DqnQueueDisc::DoDequeue ()
{
  NS_LOG_FUNCTION (this);

  
	if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();
	
	PacketProcessingRate(item, m_dequeueMeasurement, m_dequeueThreshold, m_dequeueStart, m_dequeueCount, m_dequeueRate);
	
	////std::cout << "Current Time: " << Simulator::Now().GetSeconds() << "s, Sojorun time: " << Simulator::Now().GetSeconds() - item->GetTimeStamp().GetSeconds() << "s, size: " << item->GetSize() << ", queue size: "<< GetCurrentSize() << std::endl;

	////std::cout << "Packet comes out! Length of Queue: " << GetCurrentSize () << std::endl;

	return item;
}


bool
DqnQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("DqnQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("DqnQueueDisc cannot have packet filters");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // add a DropTail queue
      AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> >
                          ("MaxSize", QueueSizeValue (GetMaxSize ())));
    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("DqnQueueDisc needs 1 internal queue");
      return false;
    }

  return true;
}

} //namespace ns3
