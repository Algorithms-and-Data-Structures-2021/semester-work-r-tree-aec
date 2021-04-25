#include <stdio.h>
#include "data_structure.hpp"

using namespace itis;

RTree::Rect rects[] =
    {
        RTree::Rect(0, 0, 2, 2), // xmin, ymin, xmax, ymax (for 2 dimensional RTree)
        RTree::Rect(5, 5, 7, 7),
        RTree::Rect(8, 5, 9, 6),
        RTree::Rect(7, 1, 9, 2),
    };

int nrects = sizeof(rects) / sizeof(rects[0]);

RTree::Rect search_rect(6, 4, 10, 6); // search will find above rects that this one overlaps


bool SearchCallback(int id, void* arg)
{
  printf("Hit data rect %d\n", id);
  return true; // keep going
}


int main() {
  itis::RTree tree;

  int i, nhits;
  printf("nrects = %d\n", nrects);

  for (i = 0; i < nrects; i++) {
    tree.Insert(rects[i].m_min, rects[i].m_max, i);  // Note, all values including zero are fine in this version
  }

  nhits = tree.Search(search_rect.m_min, search_rect.m_max, SearchCallback, nullptr);

  printf("Search resulted in %d hits\n", nhits);

  nhits = tree.Count();
  printf("count func\n", nhits);

  tree.RemoveAll();
  nhits = tree.Search(search_rect.m_min, search_rect.m_max, SearchCallback, nullptr);

  printf("Search resulted in %d hits\n", nhits);

  printf("Hello there!");

  return 0;

}


