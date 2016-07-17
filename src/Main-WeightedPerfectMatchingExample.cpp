/* Main-WeightedPerfectMatchingExample.cpp (created on 06/06/2016 by Nicolas) */

// Usage example for the WeightedPerfectMatchingLib: a group of person wants to offer gifts to each other
// for Christmas, but to avoid a costly Christmas, they use an assignment where each participant will offer
// a gift to a single other participant, selected randomly. To avoid any problems, the random assignment
// must be such that all participant offer one gift and receive one gift. Moreover, there may be some constraints
// saying that two participants cannot be assigned to one another, for example a husband and wife or two siblings.
// Additionnally, on successive years, the random assignment should ideally avoid assigning the same recipient
// participent to the same offering participant too frequently.



#include <iostream>
#include <random>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include "wpm\WeightedPerfectMatchingLib.h"



//! Class Participant, representing a participant to the random gift assignment.
class Participant
{
private:

	unsigned int _id;		//!< Positive integer uniquely representing the participant.
	std::string _name;		//!< Name of the participant.
	std::vector<unsigned int> _impossible_recipients;	//!< Array of IDs for the participants who cannot be assigned to this participant.

public:

	//! Default ctor.
	Participant()
		: _id((unsigned int)(-1))
	{
	}

	//! Initialization ctor. Assigns a new unique ID to this participant.
	Participant(unsigned int id, const std::string &name)
		: _id(id)
		, _name(name)
	{
	}

	//! Copy ctor.
	Participant(const Participant &p)
		: _id(p._id)
		, _name(p._name)
	{
	}

	//! Function to add the specified participant in the list of impossible recipients for this participant.
	void addImpossibleRecipient(const Participant &p)
	{
		_impossible_recipients.push_back(p.getID());
	}

	inline unsigned int getID() const { return _id; }
	inline std::string getName() const { return _name; }

	//! Function to return begin and end iterators for the impossible recipients of this participant.
	void getImpossibleRecipientsIterators(std::vector<unsigned int>::const_iterator &ibegin, std::vector<unsigned int>::const_iterator &iend) const
	{
		ibegin = _impossible_recipients.begin();
		iend = _impossible_recipients.end();
	}
};



//! Class representing a group of participants
class ParticipantGroup
{
private:

	std::map<unsigned int, Participant> _participants;

public:

	//! Default ctor.
	ParticipantGroup()
	{
	}

	//! Function to add a participant to the participant group, while checking that its ID is not already in use.
	void addParticipant(Participant &p)
	{
		if(_participants.find(p.getID())!=_participants.end())
			throw std::runtime_error("Duplicate participant ID!");
		_participants[p.getID()] = p;
	}

	//! Function to specify that the two participant with id1 and id2 cannot be assigned to each other.
	void addImpossibleMutualAssignment(unsigned int id1, unsigned int id2)
	{
		Participant &p1 = _participants[id1];
		Participant &p2 = _participants[id2];
		p1.addImpossibleRecipient(p2);
		p2.addImpossibleRecipient(p1);
	}

	//! Function to get the participant with specified ID.
	const Participant& getParticipant(unsigned int id)
	{
		return _participants[id];
	}

	//! Function to return the string representation of the participant graph.
	std::string getParticipantGraphString() const
	{
		const unsigned int infinite_cost = (unsigned int)(-1);
		std::ostringstream graph_str;
		// First specify the number of participants
		unsigned int nparticipants = _participants.size();
		graph_str << "nv " << nparticipants << std::endl;
		// Loop over each participant to serialize its id and constraints
		for(std::map<unsigned int, Participant>::const_iterator it_p=_participants.begin(); it_p!=_participants.end(); ++it_p) {
			graph_str << "v " << it_p->first;
			std::vector<unsigned int>::const_iterator it_ir, it_ir_end;
			it_p->second.getImpossibleRecipientsIterators(it_ir, it_ir_end);
			for(; it_ir!=it_ir_end; ++it_ir)
				graph_str << " " << *it_ir << "(" << infinite_cost << ")";
			graph_str << "\n";
		}
		return graph_str.str();
	}
};



void main()
{
	std::srand(std::random_device{}());

	// Initialize the participants
	ParticipantGroup pg;
	pg.addParticipant(Participant(0, "Alice"));
	pg.addParticipant(Participant(1, "Aaron"));
	pg.addParticipant(Participant(2, "Becky"));
	pg.addParticipant(Participant(3, "Bobby"));
	pg.addParticipant(Participant(4, "Chloe"));
	pg.addParticipant(Participant(5, "Chris"));
	pg.addParticipant(Participant(6, "Diana"));
	pg.addParticipant(Participant(7, "David"));
	pg.addImpossibleMutualAssignment(0, 1);
	pg.addImpossibleMutualAssignment(2, 3);
	pg.addImpossibleMutualAssignment(4, 5);
	pg.addImpossibleMutualAssignment(6, 7);

	// Initialize the successive assignments
	std::string graph_str = pg.getParticipantGraphString();
	unsigned int nyears = 10;

	// Simulate an assingment by year
	unsigned int iyear = 0;
	std::string error_msg;
	ResultCode rescode;
	while(iyear<nyears) {

		// Compute the matching
		std::vector<unsigned int> matching;
		std::string updated_graph_str;
		rescode = findBestPerfectMatching(graph_str, matching, updated_graph_str);
		if(rescode!=ResCode_Success) {
			if(rescode==ResCode_KnownException)
				error_msg = updated_graph_str;
			break;
		}

		// Reverse the matching
		unsigned int nids = matching.size();
		std::vector<unsigned int> matching_inv(nids);
		for(unsigned int id=0; id<nids; ++id)
			matching_inv[matching[id]] = id;

		// Display the matching
		std::cout << "Year #" << iyear << ":" << std::endl;
		//std::cout << "Graph: '" << graph_str << "'" << std::endl;
		for(unsigned int id=0; id<nids; ++id)
			std::cout << pg.getParticipant(id).getName() << " offers to " << pg.getParticipant(matching[id]).getName() << " and receives from " << pg.getParticipant(matching_inv[id]).getName() << std::endl;
		std::cout << std::endl;

		// Prepare next iteration
		++iyear;
		graph_str = updated_graph_str;
	}

	// Display feedback in case of failure
	if(rescode!=ResCode_Success) {
		switch(rescode) {
		case ResCode_InvalidGraph:
			std::cout << "Failed to compute a matching: The graph is invalid!" << std::endl;
			break;
		case ResCode_MatchingFailure:
			std::cout << "Failed to compute a matching: The matching algorithm failed!" << std::endl;
			break;
		case ResCode_InvalidMatching:
			std::cout << "Failed to compute a matching: The resulting matching is invalid!" << std::endl;
			break;
		case ResCode_KnownException:
			std::cout << "Failed to compute a matching: Exception caught: " << error_msg << std::endl;
			break;
		case ResCode_UnknownException:
			std::cout << "Failed to compute a matching: Unknown exception!" << std::endl;
			break;
		}
	}
	else std::cout << "Successfully computed " << nyears << " successive matchings!" << std::endl;

	// Wait for an acknowledgement by the user
	std::cout << "Enter any char to continue...";
	getchar();
}
