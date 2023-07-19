
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <vector>

#include "definitions.hpp"
#include "board.hpp"

struct Openings
{
	const static char delimiter = '/';

	struct Node {
		std::vector<int> next_possible_sqs;
		Node* continuations[BOARD_SIZE];
		std::string current_name;
		
		Node() {
			current_name = "root";
			for (int i=0; i<BOARD_SIZE; i++) {
				continuations[i] = nullptr;
			}
		}

		Node(std::string name) {
			current_name = name;
			for (int i=0; i<BOARD_SIZE; i++) {
				continuations[i] = nullptr;
			}
		}
	};
	Node root_node;
	Node* current_move_node;

	Openings() {
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
				if (last_node->continuations[sq] != nullptr) {
					branch = last_node->continuations[sq];
				}
				else {
					branch = new Node(name);
					last_node->continuations[sq] = branch;
				}
				last_node->next_possible_sqs.push_back(sq);
				
				last_node = branch;
			}
		}
	}

	bool next_move(Sq* dest, std::string* name) {
		std::random_device rd;
		std::mt19937 gen(rd());
		
		int size = current_move_node->next_possible_sqs.size();
		if (!size) return false;
		std::uniform_int_distribution<int> d1(0, size-1);
		Sq first = current_move_node->next_possible_sqs[d1(gen)];
		Node* second_node = current_move_node->continuations[first];

		size = second_node->next_possible_sqs.size();
		if (!size) return false;
		std::uniform_int_distribution<int> d2(0, size-1);
		Sq second = second_node->next_possible_sqs[d2(gen)];

		current_move_node = second_node->continuations[second];

		*name = second_node->current_name;
		dest[0] = first;
		dest[1] = second;

		
		return true;
	}
};

int main() {
	Openings o;

	std::cout << o.root_node.next_possible_sqs.size() << std::endl;

	// Openings::Node n = o.root_node;
	// while (n.next_ps_index)
	// {
	// 	std::cout << n.current_name << ':' << n.next_possible_sqs[0] << std::endl;
	// 	n = *n.continuations[ n.next_possible_sqs[ 0 ] ];
	// }	
	while (true)
	{
		Sq move[2] = {0};
		std::string name;
		Start_time
		bool success = o.next_move(move, &name);

		if (success) {
			std::cout << "Opening: " << name  << ' ' << i_to_sq(move[0]) << i_to_sq(move[1]) << ' ' << End_time << std::endl;
		}
		else {
			std::cout << "opening done" << std::endl;
			break;
		}
	}

	return 0;
}
