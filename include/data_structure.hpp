#pragma once
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

using namespace std;
// Заголовочный файл с объявлением структуры данных

namespace itis {
  const int dimensions = 2;
  const int max_nodes = 10000;
  const int min_nodes = 5000;


  struct RTree
  {
   protected:
    struct Node;  // предварительное объявление

   public:
    RTree();
    virtual ~RTree();

    // Вставка записи
    void Insert(const int a_min[dimensions], const int a_max[dimensions], const int& a_dataId);

    // Удаление записи
    void Remove(const int a_min[dimensions], const int a_max[dimensions], const int& a_dataId);


    // Найти все в прямоугольнике поиска
    // a_resultCallback Функция для возврата результата. Вызов должен вернуть «true», чтобы продолжить поиск.
    // Возвращает количество найденных записей
    int Search(const int a_min[dimensions], const int a_max[dimensions], bool __cdecl a_resultCallback(int a_data, void* a_context), void* a_context);


    // Удаление всех записей из дерева
    void RemoveAll();


    // Подсчит элементов данных
    int Count();

   public:

    // Минимальный ограничивающий прямоугольник
    struct Rect
    { Rect()  {}

      Rect(int a_minX, int a_minY, int a_maxX, int a_maxY)
      {
        m_min[0] = a_minX;
        m_min[1] = a_minY;

        m_max[0] = a_maxX;
        m_max[1] = a_maxY;
      }
      int m_min[dimensions];                      // Минимальные размеры
      int m_max[dimensions];                      // Максимальные размеры
    };

   protected:
    // Это могут быть данные или другое поддерево
    // Это определяет уровень родителей.
    // Если уровень родителей равен 0, то это данные
    struct Branch
    {
      Rect m_rect;                                  // Границы
      union
      {
        Node* m_child;                              // Дочерний узел
        int m_data;                                 // Данные
      };
    };

    // Узел для каждого уровня
    struct Node
    {
      bool IsInternalNode()                         { return (level > 0); }
      bool IsLeaf()                                 { return (level == 0); }

      int m_count;
      int level;
      Branch m_branch[max_nodes];
    };

    // Список ссылок узлов для повторной вставки после операции удаления
    struct ListNode
    {
      ListNode* m_next;
      Node* m_node;
    };

    // Переменные для поиска разделителя
    struct Vars {
      int m_partition[max_nodes + 1];
      int m_total;
      int m_minFill;
      int m_taken[max_nodes + 1];
      int m_count[2];
      Rect m_cover[2];
      float m_area[2];

      Branch m_branchBuf[max_nodes + 1];
      int m_branchCount;
      Rect m_coverSplit;
      float m_coverSplitArea;
    };

    Node* LocateNode();

    void InitNode(Node* a_node);

    void InitRect(Rect* a_rect);

    // Вставляет новый прямоугольник данных в структуру
    // Рекурсивно спускается по дереву
    // Возвращает 0, если узел не был разделен. Обновляет старый узел.
    // Если узел был разделен, возвращает 1 и устанавливает указатель, на который указывает
    // new_node, чтобы указать на новый узел. Обновляет старый узел.
    // Аргумент level указывает количество шагов вверх от листа
    bool InsertRectRec(Rect* a_rect, const int& a_id, Node* a_node, Node** a_newNode, int a_level);

    // Вставляем прямоугольник данных в структуру
    // InsertRect обеспечивает разделение корня
    // возвращает 1, если корень был разделен, и 0, если нет.
    // Аргумент level указывает количество шагов вверх от листа
    // InsertRect выполняет рекурсию.
    bool InsertRect(Rect* a_rect, const int& a_id, Node** a_root, int a_level);

    // Находим наименьший прямоугольник, включающий все прямоугольники в ветвях узла
    Rect NodeCover(Node* a_node);

    // Добавляем ветку к узлу. При необходимости разделяет узел.
    // Возвращает 0, если узел не разделен. Обновляет старый узел.
    // Возвращает 1, если узел разделен, устанавливает * new_node в адрес нового узла, обновляет старый узел
    bool AddBranch(Branch* a_branch, Node* a_node, Node** a_newNode);

    // Отключает зависимый узел
    void DisconnectBranch(Node* a_node, int a_index);

    // Выбирает ветку, которая потребует наименьшего увеличения
    // в области для размещения нового прямоугольника.
    // Получим наименьшую площадь перекрывающих прямоугольников в текущем узле.
    // Для одинаковых, выбираем ту, которая была меньше
    int PickBranch(Rect* a_rect, Node* a_node);

    // Объединяем два прямоугольника в один больший, содержащий оба
    Rect CombineRect(Rect* a_rectA, Rect* a_rectB);

    // Разбиваем узел.
    // Разделяет ветви узлов и дополнительную ветвь между двумя узлами.
    // Старый узел - один из новых, поэтому создается самый новый.
    void SplitNode(Node* a_node, Branch* a_branch, Node** a_newNode);

    // Вычислить площадь прямоугольника
    float RectVolume(Rect* a_rect);

    float CalcRectVolume(Rect* a_rect);


    // Создает ветвление с ветвями от полного узла
    void GetBranches(Node* a_node, Branch* a_branch, Vars* a_parVars);

    // В качестве начальных значений для двух групп выбираем два прямоугольника, которые покрывают площадь,
    // которую можно покрыть одним прямоугольником
    // Из оставшихся прямоугольников по одному выбираем для помещения в одну из двух групп.
    // Выбираем те, у которых наибольшая разница площади
    // т.е. прямоугольник больше входит в одну группу чем в другую
    // При заполнении одной группы остальные прямоугольники включаются в другую
    void ChoosePartition(Vars* a_parVars, int a_minFill);

    // Копирует ветви из буфера в два узла в соответствии с разделителем
    void LoadNodes(Node* a_nodeA, Node* a_nodeB, Vars* a_parVars);

    void InitParVars(Vars* a_parVars, int a_maxRects, int a_minFill);

    void PickSeeds(Vars* a_parVars);

    // Помещает ветку в одну из групп
    void Classify(int a_index, int a_group, Vars* a_parVars);

    // Удаляем прямоугольник из структуры
    // Передаем указатель на Rect, id записи, ptr на корневой узел.
    // Возвращает 1, если запись не найдена, иначе 0.
    // RemoveRect позволяет удалить корень.
    bool RemoveRect(Rect* a_rect, const int& a_id, Node** a_root);

    // Удаляем прямоугольник из некорневой части индексной структуры.
    // Вызываем RemoveRect. Рекурсивно спускаемся по дереву,
    // объединяем ветви на обратном пути.
    // Возвращает 1, если запись не найдена, иначе 0.
    bool RemoveRectRec(Rect* a_rect, const int& a_id, Node* a_node, ListNode** a_listNode);

    // Решает, перекрываются ли два прямоугольника
    bool Overlap(Rect* a_rectA, Rect* a_rectB);

    // Добавляем узел в список повторной вставки. Все его ветви будут
    // повторно вставленны
    void ReInsert(Node* a_node, ListNode** a_listNode);

    // Поиск в дереве или поддереве всех узловых точек, которые перекрывают прямоугольник
    bool Search(Node* a_node, Rect* a_rect, int& a_foundCount, bool __cdecl a_resultCallback(int a_data, void* a_context), void* a_context);

    void RemoveAllRec(Node* a_node);

    void CountRec(Node* a_node, int& a_count);

    Node* root;                                    // Корень
  };

}  // namespace itis