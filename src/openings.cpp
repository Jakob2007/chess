
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <vector>

#include "definitions.hpp"
#include "openings.hpp"
#include "board.hpp"

Openings::Node::Node() {
	current_name = "root";
	for (int i=0; i<BOARD_SIZE; i++) {
		continuations[i] = nullptr;
	}
}

Openings::Node::Node(std::string name) {
	current_name = name;
	for (int i=0; i<BOARD_SIZE; i++) {
		continuations[i] = nullptr;
	}
}

Openings::Openings() {
	current_move_node = &root_node;

	std::ifstream file("openings");

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string name;
		std::getline(iss, name, delimiter);

		Node* last_node = &root_node;

		std::string element;
		while (std::getline(iss, element, delimiter)) {
			Sq sq = sq_to_i(element);
			Node* branch;
			if (last_node->continuations[sq]) {
				branch = last_node->continuations[sq];
			}
			else {
				branch = new Node(name);
				last_node->continuations[sq] = branch;
				last_node->next_possible_sqs.push_back(sq);
			}
			
			last_node = branch;
		}
	}
}

void Openings::next_move(Sq* dest, std::string* name) {
	std::random_device rd;
	std::mt19937 gen(rd());
	
	int size = current_move_node->next_possible_sqs.size();
	std::uniform_int_distribution<int> d1(0, size-1);
	Sq first = current_move_node->next_possible_sqs[d1(gen)];
	Node* node1 = current_move_node->continuations[first];

	size = node1->next_possible_sqs.size();
	std::uniform_int_distribution<int> d2(0, size-1);
	Sq second = node1->next_possible_sqs[d2(gen)];

	*name = node1->current_name;
	dest[0] = first;
	dest[1] = second;
}

void Openings::push_move(Sq sq1, Sq sq2) {
	if (!current_move_node) return;
	
	Node* node1 = current_move_node->continuations[sq1];
	if (!node1) {
		current_move_node = nullptr;
		return;
	}

	current_move_node = node1->continuations[sq2];
}

void Openings::reset() {
	current_move_node = &root_node;
}

/*
int main() {
	Openings o;

	std::cout << o.root_node.next_possible_sqs.size() << std::endl;

	// Openings::Node n = o.root_node;
	// while (n.next_ps_index)
	// {
	// 	std::cout << n.current_name << ':' << n.next_possible_sqs[0] << std::endl;
	// 	n = *n.continuations[ n.next_possible_sqs[ 0 ] ];
	// }	
	while (o.current_move_node != nullptr)
	{
		Sq move[2] = {0};
		std::string name;
		Start_time
		o.next_move(move, &name);

		std::cout << "Opening: " << name  << ' ' << i_to_sq(move[0]) << i_to_sq(move[1]) << ' ' << End_time << std::endl;

	}
	std::cout << "opening done" << std::endl;

	return 0;
}
*/
