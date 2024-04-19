#include <iostream>
#include "antsdrDevice.h"
#include "unistd.h"
#include "vector"
#include "matplotlibcpp.h"

std::vector<double> x_data,recv1_i,recv1_q;
pthread_mutex_t lock;

void get_rx_data(sdr_transfer *trans){
    pthread_mutex_lock(&lock);
    recv1_i.clear();
    recv1_q.clear();
    for(int i = 0; i < trans->length / trans->channels; i++){
        recv1_i.push_back(trans->data[(2 * trans->channels) * i]);
        recv1_q.push_back(trans->data[(2 * trans->channels) * i + 1]);
    }
    pthread_mutex_unlock(&lock);
}

namespace plt = matplotlibcpp;
int main() {
    antsdrDevice device;
    if(device.open() < 0){
        return -1;
    }

    device.set_tx_samprate(3e6);
    device.set_tx_freq(1e9);
    //if(device.set_tx_attenuation(50))

    device.start_tx(1 << 0);

    device.set_rx_samprate(3e6);
    device.set_rx_freq(1e9);
    device.set_rx_gain(50, 1);

    device.start_rx(get_rx_data, 1, NULL, 1024);

    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_time{};
    diff_time = end_time - start_time;
    for(int i = 0; i < 1024; i++){
        x_data.push_back(i);
    }
    plt::figure_size(900, 300);
    while(true){
        pthread_mutex_lock(&lock);
        diff_time = end_time - start_time;
        end_time = std::chrono::high_resolution_clock::now();
        if(diff_time.count() > 60){
            break;
        }

        if(x_data.size() == recv1_i.size()){
            plt::figure(1);
            plt::clf();
            plt::plot(x_data, recv1_i, "b");
            plt::plot(x_data, recv1_q, "g");

            plt::draw();
            plt::pause(0.001);

        }
        pthread_mutex_unlock(&lock);

        sleep(1);
    }
    device.stop_rx();
    fprintf(stdout, "Exit Program.\n");
    return 0;
}
