#pragma once

int get_num_of_metadata_lines(std::string& input_file_name);

void get_num_of_blocks_and_files(std::ifstream &f, int num_of_metadata_lines);

std::string pop_next_word(std::string& main_str);

void print_results(GRBVar * blocks_migrated, GRBVar * blocks_replicated, GRBVar * files, std::string input_file_name);

void save_results(double total_time, double solver_time, std::string print_to);

void get_info_from_input_file_name(std::string input_file_name);

int main(int argc, char * argv[]);
