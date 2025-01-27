//
// Created by xixuan on 10/10/16.
//

#include "tools.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>

namespace dbm {

    template
    class Tree_info<double>;

    template
    class Tree_info<float>;

}

namespace dbm {

    template<typename T>
    void save_tree_node(const Tree_node<T> *node, std::ofstream &out) {
        if (node == nullptr) {
            out << "#" << '\n';
        } else {
            out << node->depth << ' '
                << node->column << ' '
                << node->last_node << ' '
                << node->split_value << ' '
                << node->loss << ' '
                << node->prediction << ' '
                << node->no_training_samples << ' '
                << '\n';
            save_tree_node(node->larger, out);
            save_tree_node(node->smaller, out);
        }
    }

    template<typename T>
    bool readNextToken(int &depth, int &column, bool &last_node,
                       T &split_value, T &loss, T &prediction,
                       int &no_tr_samples, std::istream &in, bool &isNumber) {

        isNumber = false;

        if (in.eof()) return false;

        std::string line;
        std::getline(in, line);

        std::string words[100];
        size_t prev = 0, next = 0;
        int count = 0;
        while ((next = line.find_first_of(' ', prev)) != std::string::npos) {
            if (next - prev != 0) {
                words[count] = line.substr(prev, next - prev);
                count += 1;
            }
            prev = next + 1;
        }

        if (prev < line.size()) {
            words[count] = line.substr(prev);
            count += 1;
        }

        if (!count || words[0] == "==")
            return false;

        if (words[0] != "#") {
            isNumber = true;
            depth = std::stoi(words[0]);
            column = std::stoi(words[1]);
            last_node = bool(std::stoi(words[2]));
            split_value = T(std::stod(words[3]));
            loss = T(std::stod(words[4]));
            prediction = T(std::stod(words[5]));
            no_tr_samples = std::stoi(words[6]);
        }
        return true;
    }

    template<typename T>
    void load_tree_node(std::ifstream &in, Tree_node<T> *&node) {
        int depth, column, no_tr_samples;
        T split_value, loss, prediction;
        bool last_node = false;
        bool isNumber;
        if (!readNextToken(depth, column, last_node, split_value, loss, prediction, no_tr_samples, in, isNumber))
            return;
        if (isNumber) {
            node = new Tree_node<T>(depth, column, last_node, split_value, loss, prediction, no_tr_samples);
            load_tree_node(in, node->larger);
            load_tree_node(in, node->smaller);
        }
    }

}

namespace dbm {

    template<typename T>
    void delete_tree(Tree_node<T> *tree) {
        delete tree;
        tree = nullptr;
    }

}

namespace dbm {

    template<typename T>
    void print_tree_info(const dbm::Tree_node<T> *tree) {
        if (tree->last_node) {
            std::cout << "depth: " << tree->depth << ' '
                      << "column: " << tree->column << ' '
                      << "split_value: " << tree->split_value << ' '
                      << "loss: " << tree->loss << ' '
                      << "last_node: " << tree->last_node << ' '
                      << "prediction: " << tree->prediction << ' '
                      << "no_training_sample: " << tree->no_training_samples << std::endl;
            std::cout << "==========" << std::endl;
            return;
        }
        std::cout << "depth: " << tree->depth << ' '
                  << "column: " << tree->column << ' '
                  << "split_value: " << tree->split_value << ' '
                  << "loss: " << tree->loss << ' '
                  << "last_node: " << tree->last_node << ' '
                  << "prediction: " << tree->prediction << ' '
                  << "no_training_sample: " << tree->no_training_samples << std::endl;
        std::cout << "==========" << std::endl;
        print_tree_info(tree->larger);
        print_tree_info(tree->smaller);
    }

    template<typename T>
    void Tree_info<T>::get_depth(const dbm::Tree_node<T> *tree) {
        if (tree->last_node) {
            depth = std::max(depth, tree->depth);
            return;
        }
        get_depth(tree->larger);
        get_depth(tree->smaller);
    }

    template<typename T>
    void Tree_info<T>::fill(const dbm::Tree_node<T> *tree, int h) {

        std::ostringstream temporary;
        temporary << "(" << std::to_string(tree->depth) << ")";


        if (tree->last_node) {
            temporary << " " << tree->prediction;
            tree_nodes[h][tree->depth] = temporary.str();
            return;
        }

        temporary << " l:" << tree->loss << " c:" << tree->column << " v:" << tree->split_value;
        tree_nodes[h][tree->depth] = temporary.str();
        int next_higher = h - std::max(1, int(height / std::pow(2, tree->depth + 2))),
                next_lower = h + int(height / std::pow(2, tree->depth + 2));
        fill(tree->larger, next_higher);
        fill(tree->smaller, next_lower);
    }

    template<typename T>
    Tree_info<T>::Tree_info(const dbm::Tree_node<T> *tree) {

        get_depth(tree);

        height = std::pow(2, depth);

        tree_nodes = new std::string *[height];
        for (int i = 0; i < height; ++i) {
            tree_nodes[i] = new std::string[depth + 1];
        }

        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < depth + 1; ++j)
                tree_nodes[i][j] = "";
        }

        fill(tree, height / 2);

    }

    template<typename T>
    Tree_info<T>::~Tree_info() {

        for (int i = 0; i < height; ++i) {
            delete[] tree_nodes[i];
        }
        delete[] tree_nodes;

    }

    template<typename T>
    void Tree_info<T>::print() const {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < depth + 1; ++j) {
                std::cout << tree_nodes[i][j] << "\t\t";
            }
            std::cout << std::endl;
        }
    }

    template<typename T>
    void Tree_info<T>::print_to_file(const std::string &file_name) const {
        std::ofstream file(file_name.c_str());
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < depth + 1; ++j) {
                file << tree_nodes[i][j] << "\t\t";
            }
            file << std::endl;
        }
        file.close();
    }

}

namespace dbm {

    void pause_at_end(std::string words) {
        std::cout << std::endl << std::endl << words << std::endl;
        std::string stop_at_end;
        std::getline(std::cin, stop_at_end);
    }

    Time_measurer::Time_measurer() {
        begin_time = std::clock();
    }

    Time_measurer::~Time_measurer() {
        end_time = std::clock();
        std::cout << "Timer at " << this
                  << " ---> " << "Elapsed Time: " << double(end_time - begin_time) / CLOCKS_PER_SEC
                  << " seconds" << std::endl << std::endl;
    }

    template<typename T>
    inline int middles(T *uniques, int no_uniques) {
        for (int i = 0; i < no_uniques - 1; ++i) uniques[i] = (uniques[i] + uniques[i + 1]) / 2.0;
        return no_uniques - 1;
    }

    template<typename T>
    inline void shuffle(T *values, int no_values) {
        std::random_shuffle(values, values + no_values);
    }

    template<typename T>
    void make_data(const std::string &file_name, int n_samples, int n_features,
                   const int *sig_lin_inds, const T *coef_sig_lin, int n_sig_lin_feats,
                   const int *sig_quad_inds, const T *coef_sig_quad, int n_sig_quad_feats) {

        if (sig_lin_inds == NULL || coef_sig_lin == NULL || sig_quad_inds == NULL || coef_sig_quad == NULL) {

            n_sig_lin_feats = 8, n_sig_quad_feats = 8;
            int lin_inds[] = {int(n_features * 0.1), int(n_features * 0.2),
                              int(n_features * 0.3), int(n_features * 0.4),
                              int(n_features * 0.5), int(n_features * 0.6),
                              int(n_features * 0.7), int(n_features * 0.8)};
            T coef_lin[] = {-10, 10, 1, 2, 5, -5, 10, -10};
            int quad_inds[] = {int(n_features * 0.15), int(n_features * 0.25),
                               int(n_features * 0.35), int(n_features * 0.45),
                               int(n_features * 0.55), int(n_features * 0.65),
                               int(n_features * 0.75), int(n_features * 0.85)};
            T coef_quad[] = {5, -3, -10, 4, 10, -5, 1, -2};


            dbm::Matrix<float> train_data(n_samples, n_features + 1);

            for (int i = 0; i < n_samples; ++i) {
                for (int j = 0; j < n_sig_lin_feats; ++j)
                    train_data[i][n_features] += coef_lin[j] * train_data[i][lin_inds[j]];
                for (int j = 0; j < n_sig_quad_feats; ++j)
                    train_data[i][n_features] += coef_quad[j] * train_data[i][quad_inds[j]];
            }

            train_data.print_to_file(file_name);
        } else {
            dbm::Matrix<float> train_data(n_samples, n_features + 1);

            for (int i = 0; i < n_samples; ++i) {
                for (int j = 0; j < n_sig_lin_feats; ++j)
                    train_data[i][n_features] += coef_sig_lin[j] * train_data[i][sig_lin_inds[j]];
                for (int j = 0; j < n_sig_quad_feats; ++j)
                    train_data[i][n_features] += coef_sig_quad[j] * train_data[i][sig_quad_inds[j]];
            }

            train_data.print_to_file(file_name);
        }

    }

    Params set_params(const std::string &param_string, const char delimiter) {

        std::string words[100];

        size_t prev = 0, next = 0;
        int count = 0;
        while ((next = param_string.find_first_of(delimiter, prev)) != std::string::npos) {
            if (next - prev != 0) {
                words[count] = param_string.substr(prev, next - prev);
                count += 1;
            }
            prev = next + 1;
        }

        if (prev < param_string.size()) {
            words[count] = param_string.substr(prev);
            count += 1;
        }

#if _DEBUG_TOOLS
        assert(count % 2 == 0);
#endif

        Params params;

        for (int i = 0; i < count / 2; ++i) {

            // DBM
            if (words[2 * i] == "no_learners")
                params.no_learners = std::stoi(words[2 * i + 1]);
            else if (words[2 * i] == "no_candidate_feature")
                params.no_candidate_feature = std::stoi(words[2 * i + 1]);
            else if (words[2 * i] == "no_train_sample")
                params.no_train_sample = std::stoi(words[2 * i + 1]);

            else if (words[2 * i] == "display_training_progress")
                params.display_training_progress = std::stoi(words[2 * i + 1]) > 0;
            else if (words[2 * i] == "record_every_tree")
                params.record_every_tree = std::stoi(words[2 * i + 1]) > 0;
            else if (words[2 * i] == "freq_showing_loss_on_test")
                params.freq_showing_loss_on_test = std::stoi(words[2 * i + 1]);

                // CART
            else if (words[2 * i] == "max_depth")
                params.max_depth = std::stoi(words[2 * i + 1]);
            else if (words[2 * i] == "no_candidate_split_point")
                params.no_candidate_split_point = std::stoi(words[2 * i + 1]);

        }

        return params;

    }

}

// explicit instantiation of templated functions
namespace dbm {

    template void save_tree_node<double>(const Tree_node<double> *node, std::ofstream &out);

    template void save_tree_node<float>(const Tree_node<float> *node, std::ofstream &out);

    template void load_tree_node<double>(std::ifstream &in, Tree_node<double> *&node);

    template void load_tree_node<float>(std::ifstream &in, Tree_node<float> *&node);

    template void delete_tree<double>(Tree_node<double> *tree);

    template void delete_tree<float>(Tree_node<float> *tree);

    template int middles<float>(float *uniqes, int no_uniques);

    template int middles<double>(double *uniqes, int no_uniques);

    template void shuffle<int>(int *values, int no_values);

    template void shuffle<float>(float *values, int no_values);

    template void shuffle<double>(double *values, int no_values);

    template void print_tree_info<double>(const dbm::Tree_node<double> *tree);

    template void print_tree_info<float>(const dbm::Tree_node<float> *tree);

    template void make_data<double>(const std::string &file_name, int n_samples, int n_features,
                                    const int *sig_lin_inds, const double *coef_sig_lin, int n_sig_lin_feats,
                                    const int *sig_quad_inds, const double *coef_sig_quad, int n_sig_quad_feats);

    template void make_data<float>(const std::string &file_name, int n_samples, int n_features,
                                   const int *sig_lin_inds, const float *coef_sig_lin, int n_sig_lin_feats,
                                   const int *sig_quad_inds, const float *coef_sig_quad, int n_sig_quad_feats);


}
