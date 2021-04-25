#include <fstream>   // ifstream
#include <iostream>  // cout
#include <string>    // string
#include <chrono>    // high_resolution_clock, duration_cast, nanoseconds

// подключаем вашу структуру данных
#include "data_structure.hpp"

using namespace std;
using namespace itis;

// абсолютный путь до набора данных и папки проекта
static constexpr auto kDatasetPath = string_view{PROJECT_DATASET_DIR};

// Возможно стоит подумать о том что данные в таблицу будут заноситься  другом порядке и изменить порядок обработки
// папок (сначала прогнать все папки для 100, 500 и тд)
string methods[3] = {"/insert/", "/search/", "/remove/"};
string folders[10] = {"/data_1/", "/data_2/", "/data_3/", "/data_4/", "/data_5/",
                      "/data_6/", "/data_7/", "/data_8/", "/data_9/", "/data_10/"};
string files[10] = {"100.csv", "500.csv", "1000.csv", "5000.csv", "10000.csv", "25000.csv",
                    "50000.csv", "100000.csv", "500000.csv", "1000000.csv"};

// Понадобится для метода Search
bool SearchCallback(int id, void* arg)
{
  return true; // keep going
}

int main(int argc, char** argv) {
  // работа с набором данных
  const auto path = string(kDatasetPath);
  string line;
  string data = "";
  for (auto file : files) {
    for (auto folder : folders) {
      for (int i = 0; i < 10; ++i) {  // 10 раз прогоняем один и тот же csv файл

        itis::RTree r_tree;  // Создание структуры, инициализация перед тестами

        // Бенч для вставки
        auto input_file_for_insert = ifstream(path + methods[0] + folder + file);

        auto time_point_before = chrono::high_resolution_clock::now();
        while (getline(input_file_for_insert, line)) {
          RTree::Rect rect(line[1], line[2], line[3], line[4]);
          r_tree.Insert(rect.m_min, rect.m_max, line[0]);
        }
        auto time_point_after = chrono::high_resolution_clock::now();
        auto time_diff = time_point_after - time_point_before;
        long time_elapsed_ns_insert = chrono::duration_cast<chrono::nanoseconds>(time_diff).count();
        // Конец

        // Бенч для поиска
        auto input_file_for_search = ifstream(path + methods[1] + folder + file);

        time_point_before = chrono::high_resolution_clock::now();
        while (getline(input_file_for_search, line)) {
          RTree::Rect search_rect(line[1], line[2], line[3], line[4]);
          r_tree.Search(search_rect.m_min, search_rect.m_max, SearchCallback, nullptr);
        }
        time_point_after = chrono::high_resolution_clock::now();
        time_diff = time_point_after - time_point_before;
        long time_elapsed_ns_search = chrono::duration_cast<chrono::nanoseconds>(time_diff).count();
        // Конец


        // Бенч для удаления
        auto input_file_for_remove = ifstream(path + methods[2] + folder + file);

        time_point_before = chrono::high_resolution_clock::now();
        while (getline(input_file_for_remove, line)) {
          RTree::Rect remove_rect(line[1], line[2], line[3], line[4]);
          r_tree.Remove(remove_rect.m_min, remove_rect.m_max, line[0]);
        }
        time_point_after = chrono::high_resolution_clock::now();
        time_diff = time_point_after - time_point_before;
        long time_elapsed_ns_remove = chrono::duration_cast<chrono::nanoseconds>(time_diff).count();
        // Конец

        cout << time_elapsed_ns_insert << "\t" << time_elapsed_ns_search << "\t" << time_elapsed_ns_remove << "\n";

        data = " ";

        r_tree.RemoveAll();
      }
    }
  }
  return 0;
}