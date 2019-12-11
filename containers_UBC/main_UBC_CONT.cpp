#include "gurobi_c++.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>
#include <chrono>
#include "main_UBC_CONT.h"

#include <ctime>


using namespace std;
using namespace std::chrono;


//file name information:
string deduplication_level;
string depth_level;
string file_system_start;
string file_system_end;
int container_size = 0;
//end of file name information


double M_presents;
double epsilon_presents;
string input_file_name;
string benchmarks_file_name;
double model_time_limit;
bool time_limit_option = false;
int grouping_factor = 0;
int num_of_containers_after_group;
double actual_M_presents = -1;
double actual_M = -1;


double M_containers = 0;
double epsilon_containers = 0;
double containers_to_replicate = -1;
int num_of_files = 0;
int num_of_containers = 0;
string seed = "-1";
string number_of_threads = "-1";
string solution_status = "ELSE";

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
    depth_level = splitted[2];
    file_system_start = splitted[3];
    file_system_end = splitted[4];
    container_size = std::stoi(splitted[5]);

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

void get_num_of_containers_and_files(std::ifstream &f, int num_of_metadata_lines) {
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
            num_of_containers = std::stoi(content.substr(2 + content.find(": ")));
        }
    }
}

string pop_next_word(string &main_str) {
    std::string delimiter = ",";
    size_t pos = main_str.find(delimiter);
    string word_to_ret;
    if (pos == string::npos) {//no delimiter found meaning end of sentence
        word_to_ret = main_str;//copy c'tor
        main_str = "";
        return word_to_ret;
    }
    word_to_ret = main_str.substr(0, pos);
    main_str.erase(0, pos + delimiter.length());
    return word_to_ret;
}

void print_results(GRBVar *containers_migrated, GRBVar *containers_replicated, GRBVar *files, string input_file_name,
                   string print_to) {
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
    int total_containers = 0;
    //solution << "containers to migrate:" << std::endl;
    for (int i = 0; i < num_of_containers_after_group; i++) {
        if (containers_migrated[i].get(GRB_DoubleAttr_X) != 0.0) {
            //solution << i << std::endl;
            total_containers++;
        }
    }
    actual_M = total_containers;
    actual_M_presents = actual_M / (double) num_of_containers_after_group;
    actual_M_presents *= 100.0;
    /*
    solution << "-----" << std::endl;
    solution << "Containers to replicate:" << std::endl;
    for (int i = 0; i < num_of_containers_after_group; i++) {
        if (containers_replicated[i].get(GRB_DoubleAttr_X) != 0.0) {
            solution << i << std::endl;
        }
    }
    //solution << "-----" << std::endl;*/
    solution.close();
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
        << grouping_factor << ","
        << container_size << ","
        << num_of_containers_after_group << ","
        << num_of_files << ","
        << M_presents << ","
        << M_containers << ","
        << actual_M_presents << ","
        << actual_M << ","
        << epsilon_presents << ","
        << epsilon_containers << ","
        << containers_to_replicate << ","
        << ((double) containers_to_replicate) * 100.0 / ((double) num_of_containers_after_group) << ","
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
                << "arguments format is: {file name} {benchmarks output file name} {M} {epsilon} {where to write the optimization solution} {container grouping factor} {model time limit in seconds} {seed} {threads} {container size} {depth} {file_system_start} {file_system_end}"
                << endl;
        return 0;
    }
    input_file_name = string(argv[1]);
    benchmarks_file_name = string(argv[2]);
    M_presents = std::stod(string(argv[3]));
    epsilon_presents = std::stod(string(argv[4]));
    string write_solution = string(argv[5]);
    grouping_factor = std::stod(string(argv[6]));
    model_time_limit = std::stod(string(argv[7]));
    time_limit_option = model_time_limit != 0;
    seed = string(argv[8]);
    number_of_threads = string(argv[9]);

    //get_info_from_input_file_name(input_file_name);
    deduplication_level = "B";
    container_size = std::stod(string(argv[10]));
    depth_level = string(argv[11]);
    file_system_start = string(argv[12]);
    file_system_end = string(argv[13]);
    int num_of_metadata_lines = get_num_of_metadata_lines(input_file_name);
    ifstream f(input_file_name.c_str(), std::ifstream::in);
    if (!f.is_open()) {
        std::cout << "error opening file." << endl;
    }

    get_num_of_containers_and_files(f, num_of_metadata_lines);//set global vars num_of_containers and num_of_files

    GRBEnv *env = 0;
    GRBVar *containers_migrated = 0;
    GRBVar *containers_replicated = 0;
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
        model.set("Seed", seed.c_str());
        model.set("Threads", number_of_threads.c_str());

        //set the model's variables.
        num_of_containers_after_group =
                num_of_containers / grouping_factor + (num_of_containers % grouping_factor != 0);
        containers_migrated = model.addVars(num_of_containers_after_group, GRB_BINARY);
        containers_replicated = model.addVars(num_of_containers_after_group, GRB_BINARY);
        files = model.addVars(num_of_files, GRB_BINARY);

        model.update();

        int file_sn;
        int container_sn;
        int number_of_containers_in_file_line;
        string current_word;
        string content;

        while (std::getline(f, content)) {
            current_word = pop_next_word(content);
            if (current_word == "F") {
                file_sn = std::stoi(pop_next_word(content));
                pop_next_word(content);//skip file_id its useless
                pop_next_word(content);//skip dir_sn its useless
                number_of_containers_in_file_line = std::stoi(pop_next_word(content));
                for (int i = 0;
                     i < number_of_containers_in_file_line; i++)//read container_sn and add constrains to the model.
                {
                    container_sn = std::stoi(pop_next_word(content));
                    pop_next_word(content);//redundent in container case, all containers are same size aprx.
                    left_side.push_back(containers_migrated[container_sn / grouping_factor] - files[file_sn]);
                    left_side.push_back(files[file_sn] - containers_migrated[container_sn / grouping_factor] -
                                        containers_replicated[container_sn / grouping_factor]);
                }
                if (number_of_containers_in_file_line == 0) {
                    left_side_hint.push_back(files[file_sn]);
                }
            } else if (current_word == "B") {
                GRBLinExpr no_orphans = 0.0;
                for (int gf_idx = 0; gf_idx < grouping_factor; ++gf_idx) {
                    container_sn = std::stoi(pop_next_word(content));
                    if (gf_idx == 0) {
                        no_orphans += containers_replicated[container_sn / grouping_factor];
                    }
                    pop_next_word(content);//skip block_id its useless
                    number_of_containers_in_file_line = std::stoi(pop_next_word(
                            content));//number of files in line reusing number_of_blocks_in_file_line for convenient
                    no_orphans -= number_of_containers_in_file_line;
                    for (int i = 0; i <
                                    number_of_containers_in_file_line; i++)//read block_sn and block_size simultaneously and add constrains to the model.
                    {
                        file_sn = std::stoi(pop_next_word(content));
                        no_orphans += files[file_sn];
                    }
                    if (gf_idx == grouping_factor - 1) break;
                    std::getline(f, content);
                    current_word = pop_next_word(content);
                    if (current_word != "B") break;
                }

                left_side.push_back(no_orphans);




                /*
                GRBLinExpr no_orphans = 0.0;
                container_sn = std::stoi(pop_next_word(content));
                pop_next_word(content);//skip block_id its useless
                number_of_containers_in_file_line = std::stoi(pop_next_word(content));//number of files in line reusing number_of_blocks_in_file_line for convenient
                no_orphans=no_orphans+containers_replicated[container_sn / grouping_factor]-number_of_containers_in_file_line;
                for (int i = 0; i <
                                number_of_containers_in_file_line; i++)//read block_sn and block_size simultaneously and add constrains to the model.
                {
                    file_sn = std::stoi(pop_next_word(content));
                    no_orphans+=files[file_sn];
                    
                }
                left_side.push_back(no_orphans);
                */
            } else {
                break;
            }
        }
        f.close();
        std::cout << "done reading the file" << endl;

        /*for (int i = 0;
             i < num_of_containers_after_group; i++)// a container cannot be migrated and replicated at the same time
        {
            left_side.push_back(containers_migrated[i] + containers_replicated[i] - 1);
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



        //del this after
        /*
        for (int i = 0; i < num_of_containers_after_group; ++i)
        {
            cout<<"m"<<i<<" = "<<(GRBVar)containers_migrated[i]<<endl;
            cout<<"r"<<i<<" = "<<(GRBVar)containers_replicated[i]<<endl;            
        }
        for (int i = 0; i < num_of_files; ++i)
        {
            cout<<"x"<<i<<" = "<<(GRBVar)files[i]<<endl;       
        }
        
        cout<<"constraints: "<<endl;
        for (unsigned int i = 0; i < left_side.size(); i++) {
            cout<<left_side[i]<<endl;
        }
        */
        //end of del



        left_side.clear();
        left_side_hint.clear();
        right_side.clear();
        right_side_hint.clear();
        names.clear();
        names_hint.clear();
        senses.clear();
        senses_hint.clear();
        //done adding to model the constrains


        GRBLinExpr all_migrated_containers = 0.0;
        GRBLinExpr all_replicated_containers = 0.0;
        for (int i = 0; i < num_of_containers_after_group; i++) {
            all_migrated_containers += containers_migrated[i];
            all_replicated_containers += containers_replicated[i];
        }
        M_containers = num_of_containers_after_group * M_presents / 100;//assign the number of bytes to migrate
        epsilon_containers = num_of_containers_after_group * epsilon_presents / 100;//assign the epsilon in bytes.


        model.addConstr(all_migrated_containers <= M_containers + epsilon_containers,
                        "5");// sum of the migrated containers should be equal to M+- epsilon.
        model.addConstr(all_migrated_containers >= M_containers - epsilon_containers,
                        "5");// sum of the migrated containers should be equal to M+- epsilon.
        model.setObjective(all_replicated_containers, GRB_MINIMIZE);//minimize the sum of replicated content.


        std::cout << "start optimize now..." << endl;
        auto s1 = high_resolution_clock::now();
        model.optimize();
        double solver_time = duration<double>(high_resolution_clock::now() - s1).count();

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
            containers_to_replicate = model.get(GRB_DoubleAttr_ObjVal);
            //print the results.
            try{
				print_results(containers_migrated, containers_replicated, files, input_file_name, write_solution);
            }
            catch (...) {
                std::cout << "Exception at print_results, probably can't read variables"<< std::endl;
                solution_status="TIME_LIMIT_AT_PRESOLVE";
            }
        }

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
    delete[] containers_migrated;
    delete[] containers_replicated;
    delete[] files;
    delete env;
    return 0;
}