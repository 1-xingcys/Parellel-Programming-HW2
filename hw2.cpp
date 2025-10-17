#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <string>
#include <sstream>
#include "image.hpp"
#include "sift.hpp"

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  std::ios_base::sync_with_stdio(false);
  std::cin.tie(NULL);

  if (argc != 4) {
    if (rank == 0) {
      std::cerr << "Usage: ./hw2 ./testcases/xx.jpg ./results/xx.jpg "
                   "./results/xx.txt\n";
    }
    MPI_Finalize();
    return 1;
  }

  std::string input_img = argv[1];
  std::string output_img = argv[2];
  std::string output_txt = argv[3];

  auto start = std::chrono::high_resolution_clock::now();

  Image img(input_img);
  img = img.channels == 1 ? img : rgb_to_grayscale(img);

  std::vector<Keypoint> kps = find_keypoints_and_descriptors(img);

  // Only rank 0 performs I/O operations
  if (rank == 0) {
    /////////////////////////////////////////////////////////////
    // The following code is for the validation
    // You can not change the logic of the following code, because it is used
    // for judge system
    std::ofstream ofs(output_txt);
    if (!ofs) {
      std::cerr << "Failed to open " << output_txt << " for writing.\n";
    } else {
      ofs << kps.size() << "\n";
#pragma omp parallel for
      for (size_t i = 0; i < kps.size(); ++i) {
        const auto &kp = kps[i];
        std::stringstream ss;
        ss << kp.i << " " << kp.j << " " << kp.octave << " " << kp.scale;
        for (size_t j = 0; j < kp.descriptor.size(); ++j) {
          ss << " " << static_cast<int>(kp.descriptor[j]);
        }
        ss << "\n";
#pragma omp critical
        { ofs << ss.str(); }
      }
      ofs.close();
    }

    Image result = draw_keypoints(img, kps);
    result.save(output_img);
    /////////////////////////////////////////////////////////////

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Execution time: " << duration.count() << " ms\n";

    std::cout << "Found " << kps.size() << " keypoints.\n";
  }

  MPI_Finalize();
  return 0;
}