/*
 * node.cc
 *
 *  Created on: 17 ��� 2020
 *      Author: gtsou
 */

#include <string.h>
#include <omnetpp.h>
#include "floodPacket_m.h"
#include <random>
#include <fstream>

using namespace omnetpp;

int transmissionCounter = 0;//����� ��������� ������� ������

class node : public cSimpleModule
{
  protected:
    // The following redefined virtual function holds the algorithm.
    virtual void initialize() override;//������������
    virtual void handleMessage(cMessage *msg) override;//�� �� ������ ���� ��� ����� ��� ������
    virtual void finish() override;
    virtual bool isNetworkConnected();
    int startingNode;//� ������ ��� �� ��������� �� flooding
    cMessage* timer;//��������� ������ ��� �������� ������ ��������� ��� ��� �����
    bool receivedPacket=false;//�� �� ��� �� ������ �� ��� �� ����������
    cTopology topo;

    //Ορισμος μιας στατικής μεταβλητής για το αριθμο των κόμβων που ελαβαν πακέτο
  public:
    static int numNodesReceived;
};

Define_Module(node); //���� ����� �� �� NED

int node::numNodesReceived = 0;

void node::initialize()
{
    startingNode = getAncestorPar("startingNode").intValue();
    if (getIndex()==startingNode){
        isNetworkConnected();
        timer = new cMessage();
        scheduleAt(simTime()+1, timer);
    }
}

void node::handleMessage(cMessage *msg)
{
        std::random_device rd;  // Obtain a random seed from the hardware
        std::mt19937 gen(rd());  // Seed the random number generator

        std::uniform_real_distribution<> dist(0.0, 1.0);  // Define the range

        double randomNum = dist(gen);

        double probability  = 0.2;//Here we define our probability for flooding;

        /*Στην ασκηση θα πρεπει να βαλουμε πιθανοτητα 0,2 0,5 και 0,9
         * ξεκινάμε με πιθανοτητα 0,2*/

    if (msg->isSelfMessage()){//����� � startingNode
        char name[50];
        sprintf(name, "Node %d packet", getIndex());
        FloodPacket* packet = new FloodPacket(name); //���������� ������� ��� flooding
        packet->setSender(getIndex());
        for (int i=gateCount()/2;i<gateCount();i++){
            send(packet->dup(), "gate$o", i - gateCount()/2);
            transmissionCounter++;
        }
        delete packet;
        receivedPacket = true;
        numNodesReceived++;
    }
    else{
        if (receivedPacket)
            delete msg;
        else{
            FloodPacket* Packet = (FloodPacket*)msg;
            for (int i=gateCount()/2;i<gateCount();i++){
                if (gateByOrdinal(i)->getNextGate()->getOwnerModule()->getIndex() != Packet->getSender()){
                    if(randomNum < probability){
                    FloodPacket* PacketDup = Packet->dup();
                    PacketDup->setSender(getIndex());
                    send(PacketDup, "gate$o", i - gateCount()/2);
                    transmissionCounter++;
                    }
                }
            }
            delete msg;
            receivedPacket=true;
            numNodesReceived++;
        }
    }
}

bool node::isNetworkConnected(){

    topo.extractByProperty("node");
    topo.calculateUnweightedSingleShortestPathsTo(topo.getNode(0));
    int rc_value = 100;
    for (int i=1;i<topo.getNumNodes();i++){
        std::cout << "LOOP" << topo.getNode(i)->getModule()->getIndex() <<endl;
//        rc_value++;
        if (topo.getNode(i)->getNumPaths() == 0)
            {
                std::cout << "NOT CONNECTED";
                return false;
                error("NOT CONNECTED");
            }
    }
//    std::cout << "The minimum value for rc is: " << "Hello" << endl;
    std::cout << "CONNECTED";
    return true;
}

void node::finish(){

    if (getIndex() == startingNode){
        delete timer;
        recordScalar("Packet transmissions", transmissionCounter);
        recordScalar("Network Coverage", (numNodesReceived / (float)topo.getNumNodes()) * 100.0);
        std::cout << "Packet transmissions  " << transmissionCounter <<endl;

        std::cout << "Number of nodes who reseived a package  " << numNodesReceived << endl;

        std::cout << "Network Coverage  " << (numNodesReceived / (float)topo.getNumNodes()) * 100.0 << "%" <<endl;

        //Output on a file
            std::ofstream outputFile("C:\\mathima\\randomTopologyAndStats\\results\\Probability0.txt");  // Open the file for writing

                if (outputFile.is_open()) {
                    outputFile << "Packet transmissions " << transmissionCounter <<std::endl << std::endl;  // Write data to the file

                    outputFile << "Number of nodes who reseived a package " << numNodesReceived << std::endl << std::endl;

                    outputFile << "Network Coverage " << (numNodesReceived / (float)topo.getNumNodes()) * 100.0 << "%" <<std::endl;

                    outputFile.close();  // Close the file
                    std::cout << "Data has been written to the: " << "Probability0.txt" << " File" << std::endl;
                } else {
                    std::cout << "Failed to open the file." << std::endl;
                }
    }

}

