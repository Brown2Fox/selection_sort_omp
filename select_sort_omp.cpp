/**
 *
 *
 */

#include <omp.h>
#include <array>
#include <vector>
#include <fstream>
#include <cassert>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <limits>
#include <iterator>

using std::vector;
using std::string;
using std::cout;
using std::endl;

namespace pr {

    struct OPTS {
        std::string in_file_name;
        std::string out_file_name;
        bool verbosity = false;
        int num_threads = 4;
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

        const char* opt_string = "i:o:vj:";

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

                case 'j':
                    opts.num_threads = strtol(optarg, nullptr, 10);
                default:
                    break;
            }
        }
    }
}

class Record
{
    static const size_t DATA_SIZE = 200;
public:
    Record() noexcept
        : valid_{ false }
        , first{ -1 }
        , second{new char[DATA_SIZE]}
    {
        second[0] = '\0';
    }

    Record(const Record& other)
        : valid_{other.valid_}
        , first{other.first}
        , second{new char[DATA_SIZE]}
    {
        std::copy(other.second, other.second + DATA_SIZE, second);
    }

    Record(Record&& other) noexcept
            : valid_{other.valid_}
            , first{other.first}
            , second{other.second}
    {
        other.second = nullptr;
    }

    Record& operator=(Record&& other) noexcept
    {
        if (this != &other)
        {
            delete[] second;

            valid_ = other.valid_;
            first = other.first;
            second = other.second;

            other.second = nullptr;
        }
        return *this;
    }

    Record& operator=(const Record& other)
    {
        if (this != &other)
        {
            std::copy(other.second, other.second + DATA_SIZE, second);

            valid_ = other.valid_;
            first = other.first;
        }
        return *this;
    }

    ~Record()
    {
        delete [] second;
    }

    bool valid() const noexcept
    {
        return valid_;
    }


    friend std::ostream& operator<< ( std::ostream&, const Record& );
    friend std::istream& operator>> ( std::istream&, Record& );

    long long int first;
    char* second;
private:
    bool valid_;
};

std::ostream& operator<< ( std::ostream& out, const Record& r )
{
    return out << "%%%" << r.first << r.second << "$$$";
}

std::istream& operator>> ( std::istream& in, Record& r )
{
    /* Read '%%%(\d+)(.*)$$$' */

    int c = in.get();

    /* Skip spaces */
    while( isspace( c ) ) {
        c = in.get();
    }

    /* Start marker '%%%' */
    for( int i = 0; i < 3; ++i ) {
        if( c != '%' ) {
            return in;
        }
        c = in.get();
    }

    /* Key, positive integer <= INT_MAX */
    if( !isdigit( c ) ) {
        return in;
    }

    r.first = 0;
    while( isdigit( c ) ) {
        r.first = r.first * 10 + ( c - '0' );
        c = in.get();
    }

    /* Value, may be empty, may contain '%%' and '$$' */
    int s_count = 0;
    int val_len = 0;
    for(;;) 
    {
        if( c == '$' ) { ++s_count; }
        else if( s_count < 3 ) { s_count = 0; }
        else { in.unget(); break; }

        r.second[val_len++] = static_cast<char>(c);

        assert(val_len < Record::DATA_SIZE && "Data size is over than 200 bytes");

        c = in.get();
    }

    /* Cut end marker '$$$' */
    r.second[val_len - 3] = '\0';

    r.valid_ = true;
    return in;
}

void read_data(std::ifstream& in, vector<Record>& data)
{
    pr::log << "Reading..." << endl;
    for( ;; ) {
        Record r{};
        in >> r;
        if( !r.valid() )
        {
            break;
        }
        data.push_back( std::move(r) );
    }
    pr::log << "done." << endl;
}

void write_data(std::ofstream& out, vector<Record>& data)
{
    pr::log << "Writing..." << endl;
    std::copy( data.begin(), data.end(), std::ostream_iterator<Record>( out, "\n" ) );
    pr::log << "done." << endl;
}

namespace std
{

    void swap(Record& x, Record& y)
    {
        Record z = std::move(x);
        x = std::move(y);
        y = std::move(z);
    };

}

using Compare = struct { size_t idx; long long int val;  };
#pragma omp declare reduction(MIN : Compare : \
    omp_out = (omp_in.val < omp_out.val) ? omp_in : omp_out) \
    initializer(omp_priv = Compare{0, std::numeric_limits<long long int>::max()})

void __selection_sort(vector<Record>& data, vector<Record>& outp, size_t start, size_t end)
{
    std::copy(&data[start], &data[end+1], &outp[start]);


    for (size_t idx_curr = start; idx_curr < end; idx_curr++)
    {
        Compare min{idx_curr, outp[idx_curr].first};

        #ifdef INNER_PARALLEL
            #pragma omp parallel for reduction(MIN: min)
        #endif
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

void selection_sort(vector<Record>& data, vector<Record>& outp)
{
    pr::log << "Sorting..." << endl;
    outp.resize(data.size(), Record{});
    __selection_sort(data, outp, 0, data.size()-1);
    pr::log << "done." << endl;
}

void parallel_selection_sort(vector<Record>& data, vector<Record>& outp)
{   
    auto data_size = data.size();
    vector<Record> partial_sorted(data_size, Record{});

    omp_set_dynamic(0);
    int num_threads = pr::opts.num_threads;

    if (data_size <= 2*2*2*2) { selection_sort(data, outp); return; }

    size_t chunk_sz = data_size / num_threads;
    size_t tail_sz = data_size % num_threads;

    vector<size_t> starts{}, ends{};

    for (auto thread_num = 0; thread_num < num_threads; thread_num++)
    {
        auto start = chunk_sz*thread_num;
        auto is_tail = thread_num == num_threads-1;
        auto end = start + (chunk_sz-1) + (is_tail ? tail_sz : 0);
        starts.push_back(start);
        ends.push_back(end);
    }

    pr::log << "Sorting..." << endl;
    #pragma omp parallel num_threads(num_threads)
    {
        auto thread_num = omp_get_thread_num();
        __selection_sort(data, partial_sorted, starts[thread_num], ends[thread_num]);
    };
    pr::log << "done." << endl;


    pr::log << "Merging..." << endl;
    for (;;)
    {
        bool is_end = true;
        for (auto thread_num = 0; thread_num < num_threads; thread_num++)
        {
            is_end = is_end && (starts[thread_num] > ends[thread_num]);
        }
        if (is_end) break;

        Compare min{};
        for (auto thread_num = 0; thread_num < num_threads; thread_num++)
        {
            if (starts[thread_num] <= ends[thread_num])
            {
                min.idx = static_cast<size_t>(thread_num);
                min.val = partial_sorted[starts[thread_num]].first;
            }
        }

        for (size_t thread_num = 0; thread_num < num_threads; thread_num++)
        {
            if (starts[thread_num] <= ends[thread_num] && partial_sorted[starts[thread_num]].first < min.val)
            {
                min.idx = thread_num;
                min.val = partial_sorted[starts[thread_num]].first;
            }
        }

        outp.push_back(std::move(partial_sorted[starts[min.idx]]));
        starts[min.idx]++;
    }
    pr::log << "done." << endl;
}






int main(int argc, char* argv[])
{
    pr::parse_args(argc, argv);

    #ifdef INNER_PARALLEL
        pr::log << "--- INNER_PARALLEL" << endl;
    #endif

    try
    {
        vector<Record> data{};
        vector<Record> sorted_data{};

        std::ifstream in{pr::opts.in_file_name};
        read_data(in, data);

        pr::log << "Data size: " << data.size() << endl;

        parallel_selection_sort(data, sorted_data);

        pr::log << "Sorted Data size: " << sorted_data.size() << endl;

        std::ofstream out{pr::opts.out_file_name};
        write_data(out, sorted_data);
    }
    catch (std::exception& e)
    {
        std::cerr << "--- " << e.what() << std::endl;
    }


    return 0;
}