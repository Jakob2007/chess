
#ifndef openings_h
#define openings_h

#include <iostream>
#include <vector>

#include "definitions.hpp"

struct Openings
{
	const char delimiter = '/';

	struct Node {
		std::vector<int> next_possible_sqs;
		Node* continuations[BOARD_SIZE];
		std::string current_name;
		
		Node();

		Node(std::string name);
	};
	Node root_node;
	Node* current_move_node;

	Openings();

	void next_move(Sq* dest, std::string* name);
	
	void push_move(Sq sq1, Sq sq2);

	void reset();
};

#endif