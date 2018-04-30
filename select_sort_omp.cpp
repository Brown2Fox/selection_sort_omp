#include <omp.h>
#include <array>
#include <vector>
#include <string>
#include <fstream>
#include <cassert>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <limits>

using std::vector;
using std::string;
using std::cout;
using std::endl;

namespace prog {

    struct OPTS {
        std::string in_file_name;
        std::string out_file_name;
        bool verbosity = false;
    } opts;

    struct LOG {

        template<typename T>
        LOG& operator<<(T&& value) {
            if (opts.verbosity) cout << value;
            return *this;
        }

        LOG& operator<<(std::ostream& (*os)(std::ostream&)) {
            if (opts.verbosity) cout << endl;
            return *this;
        }

    } log;

    void parse_args(int argc, char *argv[])
    {
        int opt = 0;

        const char* opt_string = "i:o:v";

        while( (opt = getopt( argc, argv, opt_string )) != -1 )
        {
            switch( opt )
            {
                case 'o':
                    opts.out_file_name = optarg;
                    break;

                case 'i':
                    opts.in_file_name = optarg;
                    break;

                case 'v':
                    opts.verbosity = true;
                    break;

                default:
                    break;
            }
        }
    }
}

using Compare = struct { size_t idx; size_t val;  };
#pragma omp declare reduction(MIN : Compare : \
    omp_out = (omp_in.val < omp_out.val) ? omp_in : omp_out) \
    initializer(omp_priv = Compare{0, std::numeric_limits<size_t>::max()})

void __selection_sort(vector<std::pair<size_t,string>>& data, vector<std::pair<size_t,string>>& outp, size_t start, size_t end)
{
    for (size_t i = start; i <= end; i++)
    {
        outp[i].first = data[i].first;
        outp[i].second = data[i].second;
    }

    for (size_t idx_curr = start; idx_curr < end; idx_curr++)
    {
        Compare min{idx_curr, outp[idx_curr].first};

        //#pragma omp parallel for reduction(MIN: min)
        for (size_t j = idx_curr+1; j <= end; j++)
        {
            if (outp[j].first < min.val)
            {
                min.idx = j;
                min.val = outp[j].first;
            }
        }

        std::swap(outp[idx_curr], outp[min.idx]);
    }
}


void parallel_selection_sort(vector<std::pair<size_t,string>>& data, vector<std::pair<size_t,string>>& outp)
{
    auto num_threads = 0;
    auto data_size = data.size();
    size_t chunk = 0;
    size_t rest = 0;

    vector<std::pair<size_t,string>> partial_sorted(data_size);

    prog::log << "Sorting..." << endl;
    #pragma omp parallel
    {
        num_threads = omp_get_num_threads();
        chunk = data_size / num_threads;
        rest = data_size % num_threads;
        auto thread_num = omp_get_thread_num();
        auto start = chunk*thread_num;
        auto end = start + (chunk-1) + (thread_num == num_threads-1 ? rest : 0);
        __selection_sort(data, partial_sorted, start, end);
    };
    prog::log << "done." << endl;

    prog::log << "Merging..." << endl;
    vector<size_t> inds{};
    vector<size_t> ends{};

    for (auto thread_num = 0; thread_num < num_threads; thread_num++)
    {
        inds.push_back(chunk*thread_num);
        ends.push_back(chunk*thread_num + (chunk-1) + (thread_num == num_threads-1 ? rest : 0));
    }

    while (true)
    {
        bool is_end = true;
        for (auto thread_num = 0; thread_num < num_threads; thread_num++)
        {
            is_end = is_end && (inds[thread_num] > ends[thread_num]);
        }
        if (is_end) break;

        Compare min{};
        for (auto thread_num = 0; thread_num < num_threads; thread_num++)
        {
            if (inds[thread_num] <= ends[thread_num])
            {
                min.idx = static_cast<size_t>(thread_num);
                min.val = partial_sorted[inds[thread_num]].first;
            }
        }

        for (size_t thread_num = 0; thread_num < num_threads; thread_num++)
        {
            if (inds[thread_num] <= ends[thread_num] && partial_sorted[inds[thread_num]].first < min.val)
            {
                min.idx = thread_num;
                min.val = partial_sorted[inds[thread_num]].first;
            }
        }

        outp.push_back(std::move(partial_sorted[inds[min.idx]]));
        inds[min.idx]++;
    }
    prog::log << "done." << endl;

}

inline void selection_sort(vector<std::pair<size_t,string>>& data, vector<std::pair<size_t,string>>& outp)
{
    return __selection_sort(data, outp, 0, data.size()-1);
}


void read_data(std::ifstream& in, vector<std::pair<size_t, string>>& data)
{
    prog::log << "Reading..." << endl;
    for (string line; std::getline(in, line);)
    {
        char trash;
        std::istringstream iss{line};

        size_t key = 0;
        string value{};

        iss >> trash >> trash >> trash;
        iss >> key;
        iss >> trash;
        std::getline(iss, value, '$');

        data.emplace_back(key, value);
    }
    prog::log << "done." << endl;
}

void write_data(std::ofstream& out, vector<std::pair<size_t, string>>& data)
{
    prog::log << "Writing..." << endl;
    for (auto& p: data)
    {
        out << "%%%" << p.first << ";" << p.second << "$$$" << endl;
    }
    prog::log << "done." << endl;
}

int main(int argc, char* argv[])
{
    prog::parse_args(argc, argv);

    try
    {

        vector<std::pair<size_t, string>> data{};
        vector<std::pair<size_t, string>> sorted_data{};

        std::ifstream in{prog::opts.in_file_name};
        read_data(in, data);

        prog::log << "Data size: " << data.size() << endl;

        parallel_selection_sort(data, sorted_data);

        std::ofstream out{prog::opts.out_file_name};
        write_data(out, sorted_data);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}