//
// Created by xixuan on 10/10/16.
//

#include "model.h"

#include <cassert>
#include <iostream>

namespace dbm {

    template
    class DBM<double>;

    template
    class DBM<float>;

}

namespace dbm {

    template<typename T>
    DBM<T>::DBM(int no_learners, int no_candidate_feature, int no_train_sample):
            no_learners(no_learners), no_candidate_feature(no_candidate_feature), no_train_sample(no_train_sample) {

        learners = new Base_learner<T> *[no_learners];

        for (int i = 0; i < no_learners; ++i) {
            learners[i] = nullptr;
        }

        tree_trainer = nullptr;
    }

    template<typename T>
    DBM<T>::DBM(const std::string &param_string) {

        params = set_params(param_string);

        no_learners = params.no_learners;
        no_candidate_feature = params.no_candidate_feature;
        no_train_sample = params.no_train_sample;

        learners = new Base_learner<T> *[no_learners];

        for (int i = 0; i < no_learners; ++i) {
            learners[i] = new Tree_node<T>(0);
        }

        tree_trainer = new Tree_trainer<T>(params);
    }

    template<typename T>
    DBM<T>::~DBM() {

        for (int i = 0; i < no_learners; ++i) {
            delete learners[i];
        }
        delete[] learners;

        delete tree_trainer;

    }

    template<typename T>
    void DBM<T>::train(const Matrix<T> &train_x, const Matrix<T> &train_y) {

        int n_samples = train_y.get_height(), n_features = train_x.get_width();
        int row_inds[n_samples], col_inds[n_features];

        for (int i = 0; i < n_features; ++i)
            col_inds[i] = i;
        for (int i = 0; i < n_samples; ++i)
            row_inds[i] = i;

        if (prediction_train_data != nullptr)
            delete prediction_train_data;
        prediction_train_data = new Matrix<T>(n_samples, 1, 0);

        if (params.display_training_progress) {
            if (params.record_every_tree)
                for (int i = 0; i < no_learners; ++i) {

                    std::cout << "Learner " << "(" << learners[i]->get_type() << ") " << " No. " << i << " -> ";

                    shuffle(row_inds, n_samples);
                    shuffle(col_inds, n_features);

                    tree_trainer->train(dynamic_cast<Tree_node<T> *>(learners[i]),
                                        train_x, train_y, *prediction_train_data,
                                        row_inds, no_train_sample,
                                        col_inds, no_candidate_feature);
                    tree_trainer->prune(dynamic_cast<Tree_node<T> *>(learners[i]));
                    learners[i]->predict(train_x, *prediction_train_data);

                    tree_info = new Tree_info<T>(dynamic_cast<Tree_node<T> *>(learners[i]));
                    tree_info->print_to_file("trees/tree_" + std::to_string(i) + ".txt");
                    delete tree_info;

                }
            else
                for (int i = 0; i < no_learners; ++i) {

                    std::cout << "Learner " << "(" << learners[i]->get_type() << ") " << " No. " << i << " -> ";

                    shuffle(row_inds, n_samples);
                    shuffle(col_inds, n_features);

                    tree_trainer->train(dynamic_cast<Tree_node<T> *>(learners[i]),
                                        train_x, train_y, *prediction_train_data,
                                        row_inds, no_train_sample,
                                        col_inds, no_candidate_feature);
                    tree_trainer->prune(dynamic_cast<Tree_node<T> *>(learners[i]));
                    learners[i]->predict(train_x, *prediction_train_data);

                }
        } else {
            if (params.record_every_tree)
                for (int i = 0; i < no_learners; ++i) {

                    std::cout << '.';

                    shuffle(row_inds, n_samples);
                    shuffle(col_inds, n_features);

                    tree_trainer->train(dynamic_cast<Tree_node<T> *>(learners[i]),
                                        train_x, train_y, *prediction_train_data,
                                        row_inds, no_train_sample,
                                        col_inds, no_candidate_feature);
                    tree_trainer->prune(dynamic_cast<Tree_node<T> *>(learners[i]));
                    learners[i]->predict(train_x, *prediction_train_data);

                    tree_info = new Tree_info<T>(dynamic_cast<Tree_node<T> *>(learners[i]));
                    tree_info->print_to_file("trees/tree_" + std::to_string(i) + ".txt");
                    delete tree_info;

                }
            else
                for (int i = 0; i < no_learners; ++i) {

                    std::cout << '.';

                    shuffle(row_inds, n_samples);
                    shuffle(col_inds, n_features);

                    tree_trainer->train(dynamic_cast<Tree_node<T> *>(learners[i]),
                                        train_x, train_y, *prediction_train_data,
                                        row_inds, no_train_sample,
                                        col_inds, no_candidate_feature);
                    tree_trainer->prune(dynamic_cast<Tree_node<T> *>(learners[i]));
                    learners[i]->predict(train_x, *prediction_train_data);

                }
        }

        std::cout << std::endl;

    }

    template<typename T>
    void DBM<T>::train(const Data_set<T> &data_set) {

        Matrix<T> const &train_x = data_set.get_train_x();
        Matrix<T> const &train_y = data_set.get_train_y();
        Matrix<T> const &test_x = data_set.get_test_x();
        Matrix<T> const &test_y = data_set.get_test_y();

        Loss_function<T> loss_function;

        int n_samples = train_x.get_height(), n_features = train_x.get_width();
        int test_n_samples = test_x.get_height();
        int row_inds[n_samples], col_inds[n_features];

        for (int i = 0; i < n_features; ++i)
            col_inds[i] = i;
        for (int i = 0; i < n_samples; ++i)
            row_inds[i] = i;

        if (prediction_train_data != nullptr)
            delete prediction_train_data;

        prediction_train_data = new Matrix<T>(n_samples, 1, 0);
        test_loss_record = new T[no_learners / params.freq_showing_loss_on_test];

        Matrix<T> prediction_test_data(test_n_samples, 1, 0);

        if (params.display_training_progress) {
            if (params.record_every_tree)
                for (int i = 0; i < no_learners; ++i) {

                    std::cout << "Learner " << "(" << learners[i]->get_type() << ") " << " No. " << i << " -> ";

                    shuffle(row_inds, n_samples);
                    shuffle(col_inds, n_features);

                    tree_trainer->train(dynamic_cast<Tree_node<T> *>(learners[i]),
                                        train_x, train_y, *prediction_train_data,
                                        row_inds, no_train_sample,
                                        col_inds, no_candidate_feature);
                    tree_trainer->prune(dynamic_cast<Tree_node<T> *>(learners[i]));

                    learners[i]->predict(train_x, *prediction_train_data);
                    learners[i]->predict(test_x, prediction_test_data);

                    if (!(i % params.freq_showing_loss_on_test)) {
                        test_loss_record[i / params.freq_showing_loss_on_test] = loss_function.loss(
                                prediction_test_data, test_y, 'n');
                        std::cout << std::endl
                                  << '(' << i / 10 << ')'
                                  << " Loss on test set: "
                                  << test_loss_record[i / 10]
                                  << std::endl << std::endl;
                    }

                    tree_info = new Tree_info<T>(dynamic_cast<Tree_node<T> *>(learners[i]));
                    tree_info->print_to_file("trees/tree_" + std::to_string(i) + ".txt");
                    delete tree_info;

                }
            else
                for (int i = 0; i < no_learners; ++i) {

                    std::cout << "Learner " << "(" << learners[i]->get_type() << ") " << " No. " << i << " -> ";

                    shuffle(row_inds, n_samples);
                    shuffle(col_inds, n_features);

                    tree_trainer->train(dynamic_cast<Tree_node<T> *>(learners[i]),
                                        train_x, train_y, *prediction_train_data,
                                        row_inds, no_train_sample,
                                        col_inds, no_candidate_feature);
                    tree_trainer->prune(dynamic_cast<Tree_node<T> *>(learners[i]));

                    learners[i]->predict(train_x, *prediction_train_data);
                    learners[i]->predict(test_x, prediction_test_data);

                    if (!(i % params.freq_showing_loss_on_test)) {
                        test_loss_record[i / params.freq_showing_loss_on_test] = loss_function.loss(
                                prediction_test_data, test_y, 'n');
                        std::cout << std::endl
                                  << '(' << i / 10 << ')'
                                  << " Loss on test set: "
                                  << test_loss_record[i / 10]
                                  << std::endl << std::endl;
                    }

                }
        } else {
            if (params.record_every_tree)
                for (int i = 0; i < no_learners; ++i) {

                    std::cout << '.';

                    shuffle(row_inds, n_samples);
                    shuffle(col_inds, n_features);

                    tree_trainer->train(dynamic_cast<Tree_node<T> *>(learners[i]),
                                        train_x, train_y, *prediction_train_data,
                                        row_inds, no_train_sample,
                                        col_inds, no_candidate_feature);
                    tree_trainer->prune(dynamic_cast<Tree_node<T> *>(learners[i]));

                    learners[i]->predict(train_x, *prediction_train_data);
                    learners[i]->predict(test_x, prediction_test_data);

                    if (!(i % params.freq_showing_loss_on_test)) {
                        test_loss_record[i / params.freq_showing_loss_on_test] =
                                loss_function.loss(prediction_test_data, test_y, 'n');
                    }

                    tree_info = new Tree_info<T>(dynamic_cast<Tree_node<T> *>(learners[i]));
                    tree_info->print_to_file("trees/tree_" + std::to_string(i) + ".txt");
                    delete tree_info;

                }
            else
                for (int i = 0; i < no_learners; ++i) {

                    std::cout << '.';

                    shuffle(row_inds, n_samples);
                    shuffle(col_inds, n_features);

                    tree_trainer->train(dynamic_cast<Tree_node<T> *>(learners[i]),
                                        train_x, train_y, *prediction_train_data,
                                        row_inds, no_train_sample,
                                        col_inds, no_candidate_feature);
                    tree_trainer->prune(dynamic_cast<Tree_node<T> *>(learners[i]));

                    learners[i]->predict(train_x, *prediction_train_data);
                    learners[i]->predict(test_x, prediction_test_data);

                    if (!(i % params.freq_showing_loss_on_test)) {
                        test_loss_record[i / params.freq_showing_loss_on_test] =
                                loss_function.loss(prediction_test_data, test_y, 'n');
                    }

                }
        }

        std::cout << std::endl << "Losses on Test Set: " << std::endl;
        for (int i = 0; i < no_learners / 10; ++i)
            std::cout << "(" << i << ") " << test_loss_record[i] << ' ';
        std::cout << std::endl;


    }

    template<typename T>
    void DBM<T>::predict(const Matrix<T> &data_x, Matrix<T> &predict_y) {

        int data_height = data_x.get_height();

#if _DEBUG_MODEL
        assert(data_height == predict_y.get_height() && predict_y.get_width() == 1);
#endif

        for (int i = 0; i < data_height; ++i) predict_y[i][0] = 0;

        for (int i = 0; i < no_learners; ++i) {

            learners[i]->predict(data_x, predict_y);

        }
    }

}

namespace dbm {

    template<typename T>
    void save_dbm(const DBM<T> *dbm, std::ofstream &out) {

        out << dbm->no_learners << ' '
            << dbm->no_candidate_feature << ' '
            << dbm->no_train_sample << ' '
            << std::endl;

        for (int i = 0; i < dbm->no_learners; ++i) {

            out << "== Tree " << std::to_string(i) << " ==" << std::endl;
            dbm::save_tree_node(dynamic_cast<Tree_node<T> *>(dbm->learners[i]), out);

        }

    }

    template<typename T>
    void load_dbm(std::ifstream &in, DBM<T> *&dbm) {

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

        dbm = new DBM<T>(std::stoi(words[0]), std::stoi(words[1]), std::stoi(words[2]));

        std::getline(in, line);
        Tree_node<T> *temp;
        for (int i = 0; i < dbm->no_learners; ++i) {

            temp = nullptr;
            load_tree_node(in, temp);
            dbm->learners[i] = temp;

            std::getline(in, line);

        }

    }

}

namespace dbm {

    template void save_dbm<double>(const DBM<double> *dbm, std::ofstream &out);

    template void save_dbm<float>(const DBM<float> *dbm, std::ofstream &out);

    template void load_dbm<double>(std::ifstream &in, DBM<double> *&dbm);

    template void load_dbm<float>(std::ifstream &in, DBM<float> *&dbm);

}