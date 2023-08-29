#include "rng.h"

#include <gtest/gtest.h>
#include <tgmath.h>

#include <sstream>

#include "bytes.h"
#include "test_settings.cpp"

// stores one data set of entropy data
struct entropyData {
   public:
    double entropy;       // the stored entropy
    unsigned char lower;  // the lower bound of that entropy
    unsigned char upper;  // the upper bound of that entropy
    entropyData(double entropy, unsigned char lower, unsigned char upper) {
        this->lower = lower;
        this->upper = upper;
        this->entropy = entropy;
    }
    entropyData() = default;
};

TEST(RNGClass, returnTypes) {
    // tests if the return types of the RNG class are correct
    EXPECT_EQ(typeid(unsigned char), typeid(RNG::get_random_byte()));
    EXPECT_EQ(typeid(std::string), typeid(RNG::get_random_string(1)));
    unsigned char c[1];
    EXPECT_EQ(typeid(void), typeid(RNG::fill_random_bytes(c, 1)));
}

TEST(RNGClass, entropy_bytes) {
    // calculates the entropy of the RNG gen bytes
    // a bytemap which stores how many bytes are got of a byte value
    std::unordered_map<unsigned char, u_int64_t> bytemap{};
    // generates the bytes
    unsigned char* bytes = new unsigned char[TEST_RNG_ITERS_ENTROPY];
    RNG::fill_random_bytes(bytes, TEST_RNG_ITERS_ENTROPY);

    for (u_int64_t i = 0; i < TEST_RNG_ITERS_ENTROPY; i++) {
        // getting the random byte and storing it in the bytemap
        bytemap[bytes[i]] += 1;
    }
    delete[] bytes;
    // calculates the entropy of the gotten bytes
    double entropy{};
    for (std::pair<unsigned char, u_int64_t> key_value : bytemap) {
        // calculates the actual entropy by adding p*log2(p), with p - probability of that byte
        double num = key_value.second;
        double charprob = num / TEST_RNG_ITERS_ENTROPY;
        if (isnanl(charprob) || charprob == 0) {
            continue;
        }
        entropy += charprob * (log2(charprob));
    }
    entropy = -entropy;
    if (TEST_VERBOSE) {
        std::cout << "Entropy per byte (in bit): " << entropy << std::endl;
    }
    // checks if the entropy is near enough on the perfect entropy of 8
    EXPECT_GT(entropy, 8 - TEST_RNG_ENTROPY_ERROR);
}

TEST(RNGClass, entropy_bytes2) {
    // calculates the entropy of the RNG gen bytes
    // a bytemap which stores how many bytes are got of a byte value
    std::unordered_map<unsigned char, u_int64_t> bytemap{};
    // generates the bytes
    unsigned char* bytes = new unsigned char[TEST_RNG_ITERS_ENTROPY];
    Bytes b(bytes, TEST_RNG_ITERS_ENTROPY);
    RNG::fill_random_bytes(b, TEST_RNG_ITERS_ENTROPY);

    for (u_int64_t i = 0; i < TEST_RNG_ITERS_ENTROPY; i++) {
        // getting the random byte and storing it in the bytemap
        bytemap[b.getBytes()[i]] += 1;
    }
    // calculates the entropy of the gotten bytes
    double entropy{};
    for (std::pair<unsigned char, u_int64_t> key_value : bytemap) {
        // calculates the actual entropy by adding p*log2(p), with p - probability of that byte
        double num = key_value.second;
        double charprob = num / TEST_RNG_ITERS_ENTROPY;
        if (isnanl(charprob) || charprob == 0) {
            continue;
        }
        entropy += charprob * (log2(charprob));
    }
    entropy = -entropy;
    if (TEST_VERBOSE) {
        std::cout << "Entropy per byte (in bit): " << entropy << std::endl;
    }
    // checks if the entropy is near enough on the perfect entropy of 8
    EXPECT_GT(entropy, 8 - TEST_RNG_ENTROPY_ERROR);
}

TEST(RNGClass, gen_byte_io) {
    // test for input and output of gen_byte method
    // checks if the input is correctly handled and the output should make sense
    unsigned char mode;    // some random mode to differ between test modes
    unsigned char lower;   // lower bound
    unsigned char upper;   // upper bound
    unsigned char byte;    // return byte
    unsigned char buffer;  // buffer size
    for (int i = 0; i < TEST_RNG_ITERS_IO; i++) {
        mode = RNG::get_random_string(1)[0];
        if (mode % 3 == 0) {
            // first mode with default buffer and random bounds
            lower = RNG::get_random_string(1)[0];
            upper = RNG::get_random_string(1)[0];
            buffer = 4;
        } else if (mode % 3 == 1) {
            // second mode with default max and random buffer and min
            lower = RNG::get_random_string(1)[0];
            upper = 255;
            buffer = RNG::get_random_string(1)[0];
        } else {
            // third mode, with buffer of 1 and random bounds
            lower = RNG::get_random_string(1)[0];
            upper = RNG::get_random_string(1)[0];
            buffer = 1;
        }
        if (lower > upper || buffer == 0 || buffer > 8) {
            // should throw due to invalid input
            EXPECT_THROW(RNG::get_random_byte(lower, upper, buffer), std::logic_error);
        } else {
            // should run correctly, checks if the result is in the given range
            EXPECT_NO_THROW(byte = RNG::get_random_byte(lower, upper, buffer));
            EXPECT_LE(lower, byte);
            EXPECT_GE(upper, byte);
        }
    }
}

TEST(RNGClass, entropy_byte) {
    // calculates the entropy of the RNG gen byte
    // the entropy is calculated for many combinations of bounds to test the most cases of the generate byte functionality
    unsigned char lower = 0;                           // stores the lower bound of the generated byte
    unsigned char upper = 1;                           // stores the upper bound of the generated byte
    unsigned char buffer = TEST_RNG_BYTE_BUFFER_SIZE;  // stores the buffer size that is used for the byte generation
    unsigned char range = upper - lower;               // stores the current range aka upper - lower
    bool verbose = TEST_VERBOSE;                       // should the algorithm be verbose?

    entropyData min;    // minimum entropy data for the current range
    entropyData max;    // maximum entropy data for the current range
    bool first = true;  // true if it is the first entropy for the current range
    while (true) {
        // iterates over all possible ranges (1 - 255)
        for (int j = 0; j < TEST_RNG_VALUES_PER_RANGE; j++) {
            // loops a set number of times per range
            // in each run with a range, the lower and upper bounds are picked randomly with the other generator function
            lower = RNG::get_random_string(1)[0];  // get random lower bound
            if (lower > 255 - range) {             // if lower bound is too big it has to be set to the maximum possible value
                lower = 255 - range;
            }
            upper = lower + range;
            // setting up the bytemap
            std::unordered_map<unsigned char, u_int64_t> bytemap{};
            // generating the bytes with the current settings
            for (u_int64_t i = 0; i < TEST_RNG_ITERS_BYTE_ENTROPY; i++) {
                bytemap[RNG::get_random_byte(lower, upper, buffer)] += 1;
            }
            double entropy = log2(range);  // theoretical entropy for perfect randomness
            double actual_entropy{};
            // calculates the actual entropy by adding p*log2(p), with p - probability of that byte
            for (std::pair<unsigned char, u_int64_t> key_value : bytemap) {
                double num = key_value.second;
                double charprob = num / TEST_RNG_ITERS_BYTE_ENTROPY;
                if (isnanl(charprob) || charprob == 0) {
                    // dont calculate nan and 0 values log2(0) is undefined
                    continue;
                }
                actual_entropy += charprob * (log2(charprob));
            }
            actual_entropy = -actual_entropy;  // reverse the sign to get the correct entropy
            if (verbose) {
                // if this unittest is verbose the max and min entropy differences are saved per range
                if (first) {
                    // if it is the first iteration, the min and max entropy differences are set to the current entropy difference
                    first = false;
                    min = entropyData(entropy - actual_entropy, lower, upper);
                    max = entropyData(entropy - actual_entropy, lower, upper);
                } else if (entropy - actual_entropy < min.entropy) {
                    // new entropy difference is lower than the current minimum
                    min = entropyData(entropy - actual_entropy, lower, upper);
                } else if (entropy - actual_entropy > max.entropy) {
                    // new entropy difference is higher than the current maximum
                    max = entropyData(entropy - actual_entropy, lower, upper);
                }
            }
            // actual test, the actual entropy has to be greater than the perfect entropy - some error value
            EXPECT_GT(actual_entropy, entropy - TEST_RNG_BYTE_ENTROPY_ERROR);
        }
        if (verbose) {
            // prints the max and min entropy differences for every range
            std::cout << "[RANGE " << +range << "]\tMin: " << min.entropy << "\tMax: " << max.entropy << std::endl;
        }
        if (range == 255) {
            // last range is handled, exit unittest
            break;
        }
        // extend range, next iteration will be the first one
        first = true;
        range++;
    }
}

TEST(RNGClass, byte_buffer) {
    // checks how the buffer size affects the entropy
    // the rng is calculating a random byte depending on the range and adds the lower bound
    // that means it is not necessary to check for different bounds in one range here
    std::vector<unsigned char> buffer_sizes = {1, 2, 4, 8};
    for (int range = 1; range < 256; range++) {
        // stores the differences between the actual and expected entropy for each buffer size
        std::vector<entropyData> entropydifs;
        for (unsigned char buffer_size : buffer_sizes) {
            // setting up the bytemap
            std::unordered_map<unsigned char, u_int64_t> bytemap{};
            // generating the bytes with the current settings
            for (u_int64_t i = 0; i < TEST_RNG_ITERS_BYTE_BUFFER_ENTROPY; i++) {
                bytemap[RNG::get_random_byte(255 - range, 255, buffer_size)] += 1;
            }
            double entropy = log2(range);  // theoretical entropy for perfect randomness
            double actual_entropy{};
            // calculates the actual entropy by adding p*log2(p), with p - probability of that byte
            for (std::pair<unsigned char, u_int64_t> key_value : bytemap) {
                double num = key_value.second;
                double charprob = num / TEST_RNG_ITERS_BYTE_BUFFER_ENTROPY;
                if (isnanl(charprob) || charprob == 0) {
                    // dont calculate nan and 0 values log2(0) is undefined
                    continue;
                }
                actual_entropy += charprob * (log2(charprob));
            }
            actual_entropy = -actual_entropy;  // reverse the sign to get the correct entropy
            // actual test, the actual entropy has to be greater than the perfect entropy - some error value
            // we got some different error values depending on the buffer size
            switch (buffer_size) {
                case 1:
                    EXPECT_GT(actual_entropy, entropy - TEST_RNG_BYTE_ENTROPY_ERROR_1BUFFER);
                    break;
                case 2:
                    EXPECT_GT(actual_entropy, entropy - TEST_RNG_BYTE_ENTROPY_ERROR_2BUFFER);
                    break;
                case 4:
                case 8:
                    EXPECT_GT(actual_entropy, entropy - TEST_RNG_BYTE_ENTROPY_ERROR);
                    break;
                default:
                    FAIL() << "Buffer size not supported";
            }
            // stores the entropy data for printing later
            entropydifs.push_back(entropyData(entropy - actual_entropy, 255 - range, 255));
        }
        if (TEST_VERBOSE) {
            // prints the data
            // prints for each range the entropy difference for each buffer size
            std::cout << "[RANGE " << range << "] \t{";
            unsigned char ind = 0;
            for (entropyData ed : entropydifs) {
                std::cout << ed.entropy;
                if (ind != 3) {
                    std::cout << ", ";
                }
                ind++;
            }
            std::cout << "}" << std::endl;
        }
    }
}
