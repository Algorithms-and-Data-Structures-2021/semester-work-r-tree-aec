#include "data_structure.hpp"

namespace itis {
  RTree::RTree() {
    assert(max_nodes > min_nodes);
    assert(min_nodes > 0);
    assert(sizeof(int) == sizeof(void*) || sizeof(int) == sizeof(int));
    root = LocateNode();
    root->level = 0;
  }

  RTree::~RTree() {
    RemoveAllRec(root);
  }

  void RTree::Insert(const int *a_min, const int *a_max, const int &a_dataId) {
    for(int index=0; index < dimensions; ++index)
    {
      assert(a_min[index] <= a_max[index]);
    }

    Rect rect;

    for(int axis=0; axis < dimensions; ++axis)
    {
      rect.m_min[axis] = a_min[axis];
      rect.m_max[axis] = a_max[axis];
    }

    InsertRect(&rect, a_dataId, &root, 0);

  }

  void RTree::Remove(const int *a_min, const int *a_max, const int &a_dataId) {

    for(int index=0; index < dimensions; ++index)
    {
      assert(a_min[index] <= a_max[index]);
    }

    Rect rect;

    for(int axis=0; axis < dimensions; ++axis)
    {
      rect.m_min[axis] = a_min[axis];
      rect.m_max[axis] = a_max[axis];
    }
    RemoveRect(&rect, a_dataId, &root);
  }

  int RTree::Search(const int *a_min, const int *a_max, __cdecl bool (*a_resultCallback)(int, void *), void *a_context) {

    for(int index=0; index < dimensions; ++index)
    {
      assert(a_min[index] <= a_max[index]);
    }

    Rect rect;

    for(int axis=0; axis < dimensions; ++axis)
    {
      rect.m_min[axis] = a_min[axis];
      rect.m_max[axis] = a_max[axis];
    }

    int foundCount = 0;
    Search(root, &rect, foundCount, a_resultCallback, a_context);

    return foundCount;
  }

  void RTree::RemoveAll() {
    RemoveAllRec(root);
    root = LocateNode();
    root->level = 0;
  }

  int RTree::Count() {
    int count = 0;
    CountRec(root, count);
    return count;
  }

  RTree::Node * RTree::LocateNode() {
    Node* newNode;
    newNode = new Node;
    InitNode(newNode);
    return newNode;
  }

  void RTree::InitNode(Node *a_node) {
    a_node->m_count = 0;
    a_node->level = -1;
  }

  void RTree::InitRect(Rect *a_rect) {
    for(int index = 0; index < dimensions; ++index)
    {
      a_rect->m_min[index] = 0;
      a_rect->m_max[index] = 0;
    }
  }

  bool RTree::InsertRectRec(Rect *a_rect, const int &a_id, Node *a_node, Node **a_newNode, int a_level) {

    assert(a_rect && a_node && a_newNode);
    assert(a_level >= 0 && a_level <= a_node->level);

    int index;
    Branch branch;
    Node* otherNode;

    // Все еще выше уровня для вставки, рекурсивно спускаемся по дереву
    if(a_node->level > a_level)
    {
      index = PickBranch(a_rect, a_node);
      if (!InsertRectRec(a_rect, a_id, a_node->m_branch[index].m_child, &otherNode, a_level))
      {
        // Child не был разделен
        a_node->m_branch[index].m_rect = CombineRect(a_rect, &(a_node->m_branch[index].m_rect));
        return false;
      }
      else // Child был разделен
      {
        a_node->m_branch[index].m_rect = NodeCover(a_node->m_branch[index].m_child);
        branch.m_child = otherNode;
        branch.m_rect = NodeCover(otherNode);
        return AddBranch(&branch, a_node, a_newNode);
      }
    }
    else if(a_node->level == a_level) // Дошли до уровня для вставки. Добавили прямоугольник, при необходимости разделили
    {
      branch.m_rect = *a_rect;
      branch.m_child =  reinterpret_cast<Node*> (a_id);
      // Дочернее поле листьев содержит идентификатор записи данных
      return AddBranch(&branch, a_node, a_newNode);
    }
    else
    {
      assert(0);
      return false;
    }
  }

  bool RTree::InsertRect(Rect *a_rect, const int &a_id, Node **a_root, int a_level) {

    assert(a_rect && a_root);
    assert(a_level >= 0 && a_level <= (*a_root)->level);
    for(int index=0; index < dimensions; ++index)
    {
      assert(a_rect->m_min[index] <= a_rect->m_max[index]);
    }


    Node* newRoot;
    Node* newNode;
    Branch branch;

    if(InsertRectRec(a_rect, a_id, *a_root, &newNode, a_level))  // Разделение корня
    {
      newRoot = LocateNode();  // Делаем дерево выше и создаем новый корень
      newRoot->level = (*a_root)->level + 1;
      branch.m_rect = NodeCover(*a_root);
      branch.m_child = *a_root;
      AddBranch(&branch, newRoot, nullptr);
      branch.m_rect = NodeCover(newNode);
      branch.m_child = newNode;
      AddBranch(&branch, newRoot, nullptr);
      *a_root = newRoot;
      return true;
    }

    return false;
  }

  RTree::Rect RTree::NodeCover(Node *a_node) {

    assert(a_node);

    int firstTime = true;
    Rect rect;
    InitRect(&rect);

    for(int index = 0; index < a_node->m_count; ++index)
    {
      if(firstTime)
      {
        rect = a_node->m_branch[index].m_rect;
        firstTime = false;
      }
      else
      {
        rect = CombineRect(&rect, &(a_node->m_branch[index].m_rect));
      }
    }

    return rect;
  }

  bool RTree::AddBranch(Branch *a_branch, Node *a_node, Node **a_newNode) {

    assert(a_branch);
    assert(a_node);

    if(a_node->m_count < max_nodes)  // Сплит не понадобится
    {
      a_node->m_branch[a_node->m_count] = *a_branch;
      ++a_node->m_count;

      return false;
    }
    else
    {
      assert(a_newNode);

      SplitNode(a_node, a_branch, a_newNode);
      return true;
    }
  }

  void RTree::DisconnectBranch(Node *a_node, int a_index) {

    assert(a_node && (a_index >= 0) && (a_index < max_nodes));
    assert(a_node->m_count > 0);

    // Удаляем элемент, заменив его последним элементом, чтобы предотвратить пробелы в массиве
    a_node->m_branch[a_index] = a_node->m_branch[a_node->m_count - 1];

  }

  int RTree::PickBranch(Rect *a_rect, Node *a_node) {

    assert(a_rect && a_node);

    bool firstTime = true;
    float increase;
    float bestIncr =  static_cast<float> (-1);
    float area;
    float bestArea;
    int best;
    Rect tempRect;

    for(int index=0; index < a_node->m_count; ++index)
    {
      Rect* curRect = &a_node->m_branch[index].m_rect;
      area = CalcRectVolume(curRect);
      tempRect = CombineRect(a_rect, curRect);
      increase = CalcRectVolume(&tempRect) - area;
      if((increase < bestIncr) || firstTime)
      {
        best = index;
        bestArea = area;
        bestIncr = increase;
        firstTime = false;
      }
      else if((increase == bestIncr) && (area < bestArea))
      {
        best = index;
        bestArea = area;
        bestIncr = increase;
      }
    }
    return best;
  }

  RTree::Rect RTree::CombineRect(Rect *a_rectA, Rect *a_rectB) {

    assert(a_rectA && a_rectB);

    Rect newRect;

    for(int index = 0; index < dimensions; ++index)
    {
      newRect.m_min[index] = std::min(a_rectA->m_min[index], a_rectB->m_min[index]);
      newRect.m_max[index] = std::max(a_rectA->m_max[index], a_rectB->m_max[index]);
    }

    return newRect;
  }

  void RTree::SplitNode(Node *a_node, Branch *a_branch, Node **a_newNode) {

    assert(a_node);
    assert(a_branch);

    Vars localVars;
    Vars * parVars = &localVars;
    int level;

    // Загружаем все ветки в буфер, инициализируем старый узел
    level = a_node->level;
    GetBranches(a_node, a_branch, parVars);

    // Находим разделение
    ChoosePartition(parVars, min_nodes);

    // Помещаем ветки из буфера в 2 узла в соответствии с выбранным разделом
    *a_newNode = LocateNode();
    (*a_newNode)->level = a_node->level = level;
    LoadNodes(a_node, *a_newNode, parVars);

    assert((a_node->m_count + (*a_newNode)->m_count) == parVars->m_total);
  }

  float RTree::RectVolume(Rect *a_rect) {

    assert(a_rect);

    float volume = static_cast<float> (1);

    for(int index=0; index < dimensions; ++index)
    {
      volume *= a_rect->m_max[index] - a_rect->m_min[index];
    }

    assert(volume >= static_cast<float> (0));

    return volume;
  }

  float RTree::CalcRectVolume(Rect *a_rect) {
    return RectVolume(a_rect);
  }

  void RTree::GetBranches(Node *a_node, Branch *a_branch, Vars *a_parVars) {

    assert(a_node);
    assert(a_branch);

    assert(a_node->m_count == max_nodes);

    // Загружаем буфер branch
    for(int index=0; index < max_nodes; ++index)
    {
      a_parVars->m_branchBuf[index] = a_node->m_branch[index];
    }
    a_parVars->m_branchBuf[max_nodes] = *a_branch;
    a_parVars->m_branchCount = max_nodes + 1;

    // Вычисляем прямоугольник, содержащий все в наборе
    a_parVars->m_coverSplit = a_parVars->m_branchBuf[0].m_rect;
    for(int index=1; index < max_nodes + 1; ++index)
    {
      a_parVars->m_coverSplit = CombineRect(&a_parVars->m_coverSplit, &a_parVars->m_branchBuf[index].m_rect);
    }
    a_parVars->m_coverSplitArea = CalcRectVolume(&a_parVars->m_coverSplit);

    InitNode(a_node);
  }

  void RTree::ChoosePartition(Vars *a_parVars, int a_minFill) {

    assert(a_parVars);

    float biggestDiff;
    int group, chosen, betterGroup;

    InitParVars(a_parVars, a_parVars->m_branchCount, a_minFill);
    PickSeeds(a_parVars);

    while (((a_parVars->m_count[0] + a_parVars->m_count[1]) < a_parVars->m_total)
           && (a_parVars->m_count[0] < (a_parVars->m_total - a_parVars->m_minFill))
           && (a_parVars->m_count[1] < (a_parVars->m_total - a_parVars->m_minFill)))
    {
      biggestDiff = static_cast<float> (-1);
      for(int index=0; index<a_parVars->m_total; ++index)
      {
        if(!a_parVars->m_taken[index])
        {
          Rect* curRect = &a_parVars->m_branchBuf[index].m_rect;
          Rect rect0 = CombineRect(curRect, &a_parVars->m_cover[0]);
          Rect rect1 = CombineRect(curRect, &a_parVars->m_cover[1]);
          float growth0 = CalcRectVolume(&rect0) - a_parVars->m_area[0];
          float growth1 = CalcRectVolume(&rect1) - a_parVars->m_area[1];
          float diff = growth1 - growth0;
          if(diff >= 0)
          {
            group = 0;
          }
          else
          {
            group = 1;
            diff = -diff;
          }

          if(diff > biggestDiff)
          {
            biggestDiff = diff;
            chosen = index;
            betterGroup = group;
          }
          else if((diff == biggestDiff) && (a_parVars->m_count[group] < a_parVars->m_count[betterGroup]))
          {
            chosen = index;
            betterGroup = group;
          }
        }
      }
      Classify(chosen, betterGroup, a_parVars);
    }

    // Если одна группа заполнена, помещаем оставшиеся прямоугольники в другую.
    if((a_parVars->m_count[0] + a_parVars->m_count[1]) < a_parVars->m_total)
    {
      if(a_parVars->m_count[0] >= a_parVars->m_total - a_parVars->m_minFill)
      {
        group = 1;
      }
      else
      {
        group = 0;
      }
      for(int index=0; index<a_parVars->m_total; ++index)
      {
        if(!a_parVars->m_taken[index])
        {
          Classify(index, group, a_parVars);
        }
      }
    }

    assert((a_parVars->m_count[0] + a_parVars->m_count[1]) == a_parVars->m_total);
    assert((a_parVars->m_count[0] >= a_parVars->m_minFill) &&
           (a_parVars->m_count[1] >= a_parVars->m_minFill));
  }

  void RTree::LoadNodes(Node *a_nodeA, Node *a_nodeB, Vars *a_parVars) {

    assert(a_nodeA);
    assert(a_nodeB);
    assert(a_parVars);

    for(int index=0; index < a_parVars->m_total; ++index)
    {
      assert(a_parVars->m_partition[index] == 0 || a_parVars->m_partition[index] == 1);

      if(a_parVars->m_partition[index] == 0)
      {
        AddBranch(&a_parVars->m_branchBuf[index], a_nodeA, nullptr);
      }
      else if(a_parVars->m_partition[index] == 1)
      {
        AddBranch(&a_parVars->m_branchBuf[index], a_nodeB, nullptr);
      }
    }
  }

  void RTree::InitParVars(Vars *a_parVars, int a_maxRects, int a_minFill) {

    assert(a_parVars);

    a_parVars->m_count[0] = a_parVars->m_count[1] = 0;
    a_parVars->m_area[0] = a_parVars->m_area[1] = static_cast<float> (0);
    a_parVars->m_total = a_maxRects;
    a_parVars->m_minFill = a_minFill;
    for(int index=0; index < a_maxRects; ++index)
    {
      a_parVars->m_taken[index] = false;
      a_parVars->m_partition[index] = -1;
    }
  }

  void RTree::PickSeeds(Vars *a_parVars) {

    int seed0, seed1;
    float worst, waste;
    float area[max_nodes + 1];

    for(int index=0; index<a_parVars->m_total; ++index)
    {
      area[index] = CalcRectVolume(&a_parVars->m_branchBuf[index].m_rect);
    }

    worst = -a_parVars->m_coverSplitArea - 1;
    for(int indexA=0; indexA < a_parVars->m_total-1; ++indexA)
    {
      for(int indexB = indexA+1; indexB < a_parVars->m_total; ++indexB)
      {
        Rect oneRect = CombineRect(&a_parVars->m_branchBuf[indexA].m_rect, &a_parVars->m_branchBuf[indexB].m_rect);
        waste = CalcRectVolume(&oneRect) - area[indexA] - area[indexB];
        if(waste > worst)
        {
          worst = waste;
          seed0 = indexA;
          seed1 = indexB;
        }
      }
    }
    Classify(seed0, 0, a_parVars);
    Classify(seed1, 1, a_parVars);
  }

  void RTree::Classify(int a_index, int a_group, Vars *a_parVars) {

    assert(a_parVars);
    assert(!a_parVars->m_taken[a_index]);

    a_parVars->m_partition[a_index] = a_group;
    a_parVars->m_taken[a_index] = true;

    if (a_parVars->m_count[a_group] == 0)
    {
      a_parVars->m_cover[a_group] = a_parVars->m_branchBuf[a_index].m_rect;
    }
    else
    {
      a_parVars->m_cover[a_group] = CombineRect(&a_parVars->m_branchBuf[a_index].m_rect, &a_parVars->m_cover[a_group]);
    }
    a_parVars->m_area[a_group] = CalcRectVolume(&a_parVars->m_cover[a_group]);
    ++a_parVars->m_count[a_group];
  }

  bool RTree::RemoveRect(Rect *a_rect, const int &a_id, Node **a_root) {

    assert(a_rect && a_root);
    assert(*a_root);

    Node* tempNode;
    ListNode* reInsertList = nullptr;

    if(!RemoveRectRec(a_rect, a_id, *a_root, &reInsertList))
    {
      // Находим и удаляем элемент данных
      // Повторно вставляем все ветви из удаленных узлов
      while(reInsertList)
      {
        tempNode = reInsertList->m_node;

        for(int index = 0; index < tempNode->m_count; ++index)
        {
          InsertRect(&(tempNode->m_branch[index].m_rect),
                     tempNode->m_branch[index].m_data,
                     a_root,
                     tempNode->level);
        }

        ListNode* remLNode = reInsertList;
        reInsertList = reInsertList->m_next;
        assert(remLNode->m_node);
        delete remLNode->m_node;
        delete remLNode;
      }

      // Проверяем наличие избыточного корня (не лист, 1 ребенок) и удаляем
      if((*a_root)->m_count == 1 && (*a_root)->IsInternalNode())
      {
        tempNode = (*a_root)->m_branch[0].m_child;

        assert(tempNode);
        assert(*a_root);
        delete *a_root;
        *a_root = tempNode;
      }
      return false;
    }
    else
    {
      return true;
    }
  }

  bool RTree::RemoveRectRec(Rect *a_rect, const int &a_id, Node *a_node, ListNode **a_listNode) {

    assert(a_rect && a_node && a_listNode);
    assert(a_node->level >= 0);

    if(a_node->IsInternalNode())  // не лист
    {
      for(int index = 0; index < a_node->m_count; ++index)
      {
        if(Overlap(a_rect, &(a_node->m_branch[index].m_rect)))
        {
          if(!RemoveRectRec(a_rect, a_id, a_node->m_branch[index].m_child, a_listNode))
          {
            if(a_node->m_branch[index].m_child->m_count >= min_nodes)
            {
              // дочерний элемент удален, просто изменяем размер родительского прямоугольника
              a_node->m_branch[index].m_rect = NodeCover(a_node->m_branch[index].m_child);
            }
            else
            {
              // дочерний элемент удален, в узле недостаточно записей, удаляем узел
              ReInsert(a_node->m_branch[index].m_child, a_listNode);
              DisconnectBranch(a_node, index);
            }
            return false;
          }
        }
      }
      return true;
    }
    else // лист
    {
      for(int index = 0; index < a_node->m_count; ++index)
      {
        if(a_node->m_branch[index].m_child == reinterpret_cast<Node*> (a_id))
        {
          DisconnectBranch(a_node, index);
          return false;
        }
      }
      return true;
    }
  }

  bool RTree::Overlap(Rect *a_rectA, Rect *a_rectB) {

    assert(a_rectA && a_rectB);

    for(int index=0; index < dimensions; ++index)
    {
      if (a_rectA->m_min[index] > a_rectB->m_max[index] ||
          a_rectB->m_min[index] > a_rectA->m_max[index])
      {
        return false;
      }
    }
    return true;
  }

  void RTree::ReInsert(Node *a_node, ListNode **a_listNode) {

    ListNode* newListNode;

    newListNode = new ListNode;
    newListNode->m_node = a_node;
    newListNode->m_next = *a_listNode;
    *a_listNode = newListNode;
  }

  bool RTree::Search(Node *a_node, Rect *a_rect, int &a_foundCount, __cdecl bool (*a_resultCallback)(int, void *), void *a_context) {

    assert(a_node);
    assert(a_node->level >= 0);
    assert(a_rect);

    if(a_node->IsInternalNode()) // Это внутренний узел в дереве
    {
      for(int index=0; index < a_node->m_count; ++index)
      {
        if(Overlap(a_rect, &a_node->m_branch[index].m_rect))
        {
          if(!Search(a_node->m_branch[index].m_child, a_rect, a_foundCount, a_resultCallback, a_context))
          {
            return false;
          }
        }
      }
    }
    else // Лист
    {
      for(int index=0; index < a_node->m_count; ++index)
      {
        if(Overlap(a_rect, &a_node->m_branch[index].m_rect))
        {
          int& id = a_node->m_branch[index].m_data;

          if(&a_resultCallback)
          {
            ++a_foundCount;
            if(!a_resultCallback(id, a_context))
            {
              return false;
            }
          }
        }
      }
    }

    return true;
  }

  void RTree::RemoveAllRec(Node *a_node) {

    assert(a_node);
    assert(a_node->level >= 0);

    if(a_node->IsInternalNode()) // Это внутренний узел в дереве
    {
      for(int index=0; index < a_node->m_count; ++index)
      {
        RemoveAllRec(a_node->m_branch[index].m_child);
      }
    }
    assert(a_node);
    delete a_node;
  }


  void RTree::CountRec(Node *a_node, int &a_count) {

    if(a_node->IsInternalNode())  // не листовой узел
    {
      for(int index = 0; index < a_node->m_count; ++index)
      {
        CountRec(a_node->m_branch[index].m_child, a_count);
      }
    }
    else // листовой узел
    {
      a_count += a_node->m_count;
    }
  }
  // здесь должны быть определения методов вашей структуры

}  // namespace itis