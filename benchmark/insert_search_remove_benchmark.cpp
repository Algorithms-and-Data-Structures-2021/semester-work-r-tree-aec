#include <fstream>      // ifstream
#include <iostream>     // cout
#include <string>       // string, stoi
#include <string_view>  // string_view
#include <chrono>       // high_resolution_clock, duration_cast, nanoseconds
#include <sstream>
#include <vector>

// подключаем вашу структуру данных
#include "data_structure.hpp"

using namespace std;
using namespace itis;

// Ваш путь до проекта
const string path_to_project = "K:\\c++ projects\\semester-work-r-tree-aec-1";

// абсолютный путь до набора данных и папки проекта
static const auto kDatasetPathAdd = path_to_project + "\\dataset\\insert_remove\\";


//100, 500, 1000, 10000, 50000, 100000, 500000, 1000000
static const int kSizeDataset = 100;

bool SearchCallback(int id, void* arg)
{
  // printf("Hit data rect %d\n", id); В функции поиска выводит номера прямоугольников, пересекающихся с искомым
  return true; // keep going
}

int main() {
  // работа с набором данных
  string path = kDatasetPathAdd;



  for (int i = 1; i <= 10; i++) { // для каждого из 10 наборов(папки: 01, 02, 03 и т.д.)

      auto input_file = ifstream(path + "data_" + to_string(i) + "\\" + to_string(kSizeDataset) + ".csv");
      if (!input_file.is_open()) {
        cout << "open " << path + "data_" + to_string(i) + "\\" + to_string(kSizeDataset) + ".csv"
             << " error!" << endl;
        return -1;  // если файл не открылся, выводим ошибку
      }

      itis::RTree r_tree;  //создаем R-дерево
      long long time_elapsed_ns_insert;
      vector<int> for_remove;
      while (!input_file.eof()) {

        string line;
        input_file >> line;
        std::vector<int> vect;

        std::stringstream ss(line);
        if (line.size() == 0)
          break;
        for (int k; ss >> k;) {
          vect.push_back(k);
          if (ss.peek() == ',')
            ss.ignore();
        }
        if (vect[0] == 1)
          time_elapsed_ns_insert = 0;

        //======================================Вставка=======================================================
        auto time_point_before = chrono::steady_clock::now();
        RTree::Rect rect(vect[1], vect[2], vect[3], vect[4]);
        r_tree.Insert(rect.m_min, rect.m_max, vect[0]);
        auto time_point_after = chrono::steady_clock::now();
        auto time_diff = time_point_after - time_point_before;
        time_elapsed_ns_insert += chrono::duration_cast<chrono::nanoseconds>(time_diff).count();
        //======================================================================================================
      }
      //======================================Поиск=======================================================
      auto time_point_before = chrono::steady_clock::now();
      RTree::Rect search_rect(238130, 986192, 468585, 989623);  // рандомные числа, подходящие под диапазон
      auto hits = r_tree.Search(search_rect.m_min, search_rect.m_max, SearchCallback, nullptr);
      cout << hits << "\n";
      auto time_point_after = chrono::steady_clock::now();
      auto time_diff = time_point_after - time_point_before;
      long long time_elapsed_ns_search = chrono::duration_cast<chrono::nanoseconds>(time_diff).count();
      //===================================================================================================

      //======================================Удаление=======================================================
      time_point_before = chrono::steady_clock::now();
      r_tree.RemoveAll();
      time_point_after = chrono::steady_clock::now();
      time_diff = time_point_after - time_point_before;
      long long time_elapsed_ns_remove = chrono::duration_cast<chrono::nanoseconds>(time_diff).count();

      cout << time_elapsed_ns_insert << "\t" << time_elapsed_ns_search << "\t" << time_elapsed_ns_remove
           << "\n";

  }
  return 0;

}

