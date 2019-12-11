
#include "gurobi_c++.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>
#include <chrono>
#include "main_UBC_BLOCK.h"
#include <boost/algorithm/string.hpp>

#include <ctime>


using namespace std;
using namespace std::chrono;

//file name information:
string deduplication_level;
string depth_level;
string file_system_start;
string file_system_end;
double total_block_size_Kbytes = 0;
//end of file name information


double M_presents;
double epsilon_presents;
string input_file_name;
string benchmarks_file_name;
double model_time_limit;
bool time_limit_option = false;

double M_Kbytes = 0;
double epsilon_Kbytes = 0;
double Kbytes_to_replicate = -1;
int num_of_files = 0;
int num_of_blocks = 0;
double actual_M_presents = -1;
double actual_M_Kbytes = -1;
string seed = "-1";
string number_of_threads = "-1";
string solution_status = "ELSE";
int filter_factor = -1;
int average_block_size = -1;


vector <string> split(const string &str, const string &delim) {
    vector <string> tokens;
    size_t prev = 0, pos = 0;
    do {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token = str.substr(prev, pos - prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

void get_info_from_input_file_name(string input_file_name) {
    vector <string> splitted = split(input_file_name, "_");
    deduplication_level = splitted[0];
    file_system_start = splitted[2];
    file_system_end = splitted[3];
}


int get_num_of_metadata_lines(string &input_file_name) {
    ifstream f(input_file_name.c_str(), std::ifstream::in);
    int counter = 0;
    string content;
    if (!f.is_open()) {
        std::cout << "error opening file." << endl;
        //return 1;
    }
    std::getline(f, content);
    while (content[0] == '#') {
        counter++;
        std::getline(f, content);
    }
    f.close();
    return counter;
}

void get_num_of_blocks_and_files(std::ifstream &f, int num_of_metadata_lines) {
    const string type_of_info_file = "# Num files";
    const string type_of_info_block = "# Num Blocks";
    string content;
    string number_as_string;
    string type_of_info;

    for (int i = 0; i < num_of_metadata_lines; i++) {
        std::getline(f, content);
        type_of_info = content.substr(0, content.find(": "));
        if (type_of_info == type_of_info_file) {
            num_of_files = std::stoi(content.substr(2 + content.find(": ")));
        }
        if (type_of_info == type_of_info_block) {
            num_of_blocks = std::stoi(content.substr(2 + content.find(": ")));
        }
    }
}

/// string split
vector<string> splitString(string str, const std::string &delims) {
    string toShred(str);
    vector<string> result;
    boost::split(result, toShred, boost::is_any_of(delims));
    return result;
}


void print_results(GRBVar *blocks_migrated, GRBVar *blocks_replicated, GRBVar *files, string input_file_name,
                   string print_to, double *block_size) {
    ofstream solution(print_to, std::ios_base::app);
    if (!solution) {
        cout << "Cannot open output file" << print_to << endl;
        return;
    }
    for (int i = 0; i < num_of_files; i++) {
        if (files[i].get(GRB_DoubleAttr_X) != 0.0) {
            solution << i << std::endl;
        }
    }
    //solution << "-----" << std::endl;
    int total_blocks = 0;
    //solution << "blocks to migrate:" << std::endl;
    for (int i = 0; i < num_of_blocks; i++) {
        if (blocks_migrated[i].get(GRB_DoubleAttr_X) != 0.0) {
            //solution << i << std::endl;
            total_blocks += block_size[i];
        }
    }
    actual_M_Kbytes = (double) total_blocks;
    actual_M_presents = actual_M_Kbytes / (double) total_block_size_Kbytes;
    actual_M_presents *= 100.0;
    /*solution << "-----" << std::endl;
    solution << "blocks to replicate:" << std::endl;
    for (int i = 0; i < num_of_blocks; i++) {
        if (blocks_replicated[i].get(GRB_DoubleAttr_X) != 0.0) {
            solution << i << std::endl;
        }
    }
    solution << "-----" << std::endl;*/
    solution.close();
}

void save_block_size_array(double *block_size) {
    ofstream out("block_size.txt");
    if (!out) {
        cout << "Cannot open output file\n";
        return;
    }
    for (int i = 0; i < num_of_blocks; i++) {
        out << block_size[i] << endl;
    }
    out.close();
}

void load_block_size_array_and_del_temp_file(double *block_size) {
    ifstream in("block_size.txt");
    if (!in) {
        cout << "Cannot open input file\n";
        return;
    }
    for (int i = 0; i < num_of_blocks; i++) {
        in >> block_size[i];
    }
    in.close();

    if (remove("block_size.txt") != 0)// delete temp file
        std::cout << "Error deleting file" << endl;
}

void save_results(double total_time, double solver_time) {
    ofstream out(benchmarks_file_name, std::ios_base::app);
    if (!out) {
        cout << "Cannot open output file\n";
    }
    string is_there_time_limit = (time_limit_option) ? "yes" : "no";

    out << input_file_name << ","

        << deduplication_level << ","
        << depth_level << ","
        << file_system_start << ","
        << file_system_end << ","
        << filter_factor << ","
        << average_block_size << ","
        << num_of_blocks << ","
        << num_of_files << ","
        << M_presents << ","
        << M_Kbytes << ","
        << actual_M_presents << ","
        << actual_M_Kbytes << ","
        << epsilon_presents << ","
        << epsilon_Kbytes << ","
        << Kbytes_to_replicate << ","
        << ((double) Kbytes_to_replicate) * 100.0 / ((double) total_block_size_Kbytes) << ","
        << seed << ","
        << number_of_threads << ","
        << is_there_time_limit << ","
        << solution_status << ","
        << total_time << ","
        << solver_time << endl;
    out.close();
}

int main(int argc, char *argv[]) {
    const auto begin = high_resolution_clock::now();
    if (argc != 14) {
        cout
                << "arguments format is: {file name} {benchmarks output file name} {M} {epsilon} {where to write the optimization solution} {k filter factor} {model time limit in seconds} {seed} {threads} {avg block size} {depth} {file_system_start} {file_system_end}"
                << endl;
        return 0;
    }

    input_file_name = string(argv[1]);
    benchmarks_file_name = string(argv[2]);
    M_presents = std::stod(string(argv[3]));
    epsilon_presents = std::stod(string(argv[4]));
    string write_solution = string(argv[5]);
    filter_factor = std::stod(string(argv[6]));
    model_time_limit = std::stod(string(argv[7]));
    time_limit_option = model_time_limit != 0;
    seed = string(argv[8]);
    number_of_threads = string(argv[9]);

    //get_info_from_input_file_name(input_file_name);
    deduplication_level = "B";
    average_block_size = std::stod(string(argv[10]));
    depth_level = string(argv[11]);
    file_system_start = string(argv[12]);
    file_system_end = string(argv[13]);

    int num_of_metadata_lines = get_num_of_metadata_lines(input_file_name);
    ifstream f(input_file_name.c_str(), std::ifstream::in);
    if (!f.is_open()) {
        std::cout << "error opening file." << endl;
    }

    get_num_of_blocks_and_files(f, num_of_metadata_lines);//set global vars num_of_blocks and num_of_files

    GRBEnv *env = 0;
    GRBVar *blocks_migrated = 0;
    GRBVar *blocks_replicated = 0;
    GRBVar *files = 0;
    GRBConstr *constrains = 0;
    GRBConstr *constrains_hint = 0;
    bool need_to_free_hint_constrains = false;
    vector <GRBLinExpr> left_side;
    vector <GRBLinExpr> left_side_hint;
    try {
        env = new GRBEnv();//this throws exception
        GRBModel model = GRBModel(*env);
        model.set(GRB_StringAttr_ModelName, "migration_problem");
        if (time_limit_option) {
            model.set(GRB_DoubleParam_TimeLimit, model_time_limit);//set time limit
        }
        double *block_size = new double[num_of_blocks];//init block size array will be used later on building model's equations and objective function.
        std::fill_n(block_size, num_of_blocks, 0);

        model.set("Seed", seed.c_str());
        model.set("Threads", number_of_threads.c_str());

        //set the model's variables.
        blocks_migrated = model.addVars(num_of_blocks, GRB_BINARY);
        blocks_replicated = model.addVars(num_of_blocks, GRB_BINARY);
        files = model.addVars(num_of_files, GRB_BINARY);

        model.update();

        int file_sn;
        int block_sn;
        int number_of_blocks_in_file_line;
        string current_word;
        string content;
        int size_read;
        std::vector<string> splitted_content;
        while (std::getline(f, content)) {
            splitted_content=splitString(content, ",");
            current_word = splitted_content[0];
            if (current_word == "F") {
                file_sn = std::stoi(splitted_content[1]);
                //skip file_id its useless
                //skip dir_sn its useless
                number_of_blocks_in_file_line = std::stoi(splitted_content[4]);
                for (register int i = 0; i < 2*number_of_blocks_in_file_line; i+=2)//read block_sn and block_size simultaneously and add constrains to the model.
                {
                    block_sn = std::stoi(splitted_content[5+i]);
                    size_read = std::stoi(splitted_content[6+i]);//update block size histogram
                    if (block_size[block_sn] == 0) {
                        block_size[block_sn] = ((double) size_read) / 1024.0;
                    }
                    left_side.push_back(blocks_migrated[block_sn] - files[file_sn]);
                    left_side.push_back(files[file_sn] - blocks_migrated[block_sn] - blocks_replicated[block_sn]);
                }
                if (number_of_blocks_in_file_line == 0) {
                    left_side_hint.push_back(files[file_sn]);
                }
            } else if (current_word == "B") {
                GRBLinExpr no_orphans = 0.0;
                block_sn = std::stoi(splitted_content[1]);
                //skip block_id its useless
                number_of_blocks_in_file_line = std::stoi(splitted_content[3]);//number of files in line reusing number_of_blocks_in_file_line for convenient
                no_orphans = no_orphans + blocks_replicated[block_sn] - number_of_blocks_in_file_line;
                for (int i = 0; i <
                                number_of_blocks_in_file_line; i++)//read block_sn and block_size simultaneously and add constrains to the model.
                {
                    file_sn = std::stoi(splitted_content[4+i]);
                    no_orphans += files[file_sn];

                }
                left_side.push_back(no_orphans);
            } else {
                break;
            }
        }
        f.close();
        std::cout << "done reading the file" << endl;
        /*
        for (int i = 0; i < num_of_blocks; i++)// a block cannot be migrated and replicated at the same time
        {
            left_side.push_back(blocks_migrated[i] + blocks_replicated[i] - 1);
        }*/

        //add to the model the constrains
        vector<double> right_side;
        vector<double> right_side_hint;
        vector <string> names;
        vector <string> names_hint;
        vector<char> senses;
        vector<char> senses_hint;
        names.assign(left_side.size(), "");
        names_hint.assign(left_side_hint.size(), "");
        right_side.assign(left_side.size(), 0.0);
        right_side_hint.assign(left_side_hint.size(), 0.0);
        senses.assign(left_side.size(), GRB_LESS_EQUAL);
        senses_hint.assign(left_side_hint.size(), GRB_EQUAL);

        constrains = model.addConstrs(&left_side[0], &senses[0], &right_side[0], &names[0], (int) left_side.size());
        if ((int) left_side_hint.size() != 0) {
            constrains_hint = model.addConstrs(&left_side_hint[0], &senses_hint[0], &right_side_hint[0], &names_hint[0],
                                               (int) left_side_hint.size());
            need_to_free_hint_constrains = true;
        }


        left_side.clear();
        left_side_hint.clear();
        right_side.clear();
        right_side_hint.clear();
        names.clear();
        names_hint.clear();
        senses.clear();
        senses_hint.clear();

        //done adding to model the constrains
        GRBLinExpr all_migrated_blocks = 0.0;
        GRBLinExpr all_replicated_blocks = 0.0;
        for (int i = 0; i < num_of_blocks; i++) {
            all_migrated_blocks += blocks_migrated[i] * block_size[i];
            all_replicated_blocks += blocks_replicated[i] * block_size[i];
            total_block_size_Kbytes += block_size[i];
        }
        M_Kbytes = total_block_size_Kbytes * M_presents / 100;//assign the number of bytes to migrate
        epsilon_Kbytes = total_block_size_Kbytes * epsilon_presents / 100;//assign the epsilon in bytes.


        model.addConstr(all_migrated_blocks <= M_Kbytes + epsilon_Kbytes,
                        "5");// sum of the migrated blocks should be equal to M+- epsilon.
        model.addConstr(all_migrated_blocks >= M_Kbytes - epsilon_Kbytes,
                        "5");// sum of the migrated blocks should be equal to M+- epsilon.
        model.setObjective(all_replicated_blocks, GRB_MINIMIZE);//minimize the sum of replicated content.

        //save block_size arr into file
        //auto s1 = high_resolution_clock::now();
        save_block_size_array(block_size);
        delete[] block_size;
        //double saving_array_time=duration<double>(high_resolution_clock::now() - s1).count();
        //model.write("my_model.lp");//for DEBUG
        std::cout << "start optimize now..." << endl;
        auto s1 = high_resolution_clock::now();
        model.optimize();
        double solver_time = duration<double>(high_resolution_clock::now() - s1).count();

        block_size = new double[num_of_blocks];
        load_block_size_array_and_del_temp_file(block_size);


        int status = model.get(GRB_IntAttr_Status);
        if (status == GRB_OPTIMAL) {
            solution_status = "OPTIMAL";
        }
        if (status == GRB_INFEASIBLE) {
            solution_status = "INFEASIBLE";
        }
        if (status == GRB_TIME_LIMIT) {
            solution_status = "TIME_LIMIT";
        }
        cout << "done optimization" << endl << flush;

        if (solution_status != "INFEASIBLE") {
            Kbytes_to_replicate = model.get(GRB_DoubleAttr_ObjVal);
            //print the results.
            try{
                print_results(blocks_migrated, blocks_replicated, files, input_file_name, write_solution, block_size);      
            }
            catch (...) {
                std::cout << "Exception at print_results, probably can't read variables"<< std::endl;
                solution_status="TIME_LIMIT_AT_PRESOLVE";
            }
        }

        delete[] block_size;

        double elapsed_secs = duration<double>(high_resolution_clock::now() - begin).count();

        save_results(elapsed_secs, solver_time);
    }
    catch (...) {
        std::cout << "Exception during optimization"
                  << std::endl;
    }
    delete[] constrains;
    if (need_to_free_hint_constrains) {
        delete[] constrains_hint;
    }
    delete[] blocks_migrated;
    delete[] blocks_replicated;
    delete[] files;
    delete env;
    return 0;
}