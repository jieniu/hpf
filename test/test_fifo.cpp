#include <fifo.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(Fifo, test_push_back_pop)
{
    Fifo<int> fifo;
    fifo.push_back(1);
    fifo.push_back(2);
    fifo.push_back(3);
    EXPECT_EQ(fifo.size(), 3);

    int top = 0;
    EXPECT_EQ(fifo.pop(top), 0);
    EXPECT_EQ(top, 1);
    EXPECT_EQ(fifo.size(), 2);

    EXPECT_EQ(fifo.pop(top), 0);
    EXPECT_EQ(top, 2);
    EXPECT_EQ(fifo.size(), 1);
    
    fifo.push_back(4);
    EXPECT_EQ(fifo.size(), 2);

    EXPECT_EQ(fifo.pop(top), 0);
    EXPECT_EQ(top, 3);
    EXPECT_EQ(fifo.size(), 1);

    EXPECT_EQ(fifo.pop(top), 0);
    EXPECT_EQ(top, 4);
    EXPECT_EQ(fifo.size(), 0);

    EXPECT_EQ(fifo.pop(top), -1);

}

TEST(Fifo, test_push_front_pop)
{
    Fifo<int> fifo;
    fifo.push_front(1);
    fifo.push_front(2);
    fifo.push_front(3);
    EXPECT_EQ(fifo.size(), 3);

    int top = 0;
    EXPECT_EQ(fifo.pop(top), 0);
    EXPECT_EQ(top, 3);
    EXPECT_EQ(fifo.size(), 2);

    EXPECT_EQ(fifo.pop(top), 0);
    EXPECT_EQ(top, 2);
    EXPECT_EQ(fifo.size(), 1);

    fifo.push_front(4);
    EXPECT_EQ(fifo.size(), 2);

    EXPECT_EQ(fifo.pop(top), 0);
    EXPECT_EQ(top, 4);
    EXPECT_EQ(fifo.size(), 1);

    EXPECT_EQ(fifo.pop(top), 0);
    EXPECT_EQ(top, 1);
    EXPECT_EQ(fifo.size(), 0);

    EXPECT_EQ(fifo.pop(top), -1);
}

TEST(Fifo, test_erase)
{
    // case 1: no node
    Fifo<int> fifo;
    EXPECT_EQ(-1, fifo.erase_latest(1));
    EXPECT_EQ(0, fifo.size());

    // case 2: pop all
    fifo.push_back(1);
    fifo.push_back(3);
    int top;
    EXPECT_EQ(2, fifo.size());
    EXPECT_EQ(0, fifo.pop(top));
    EXPECT_EQ(1, fifo.size());
    EXPECT_EQ(0, fifo.pop(top));
    EXPECT_EQ(0, fifo.size());
    EXPECT_EQ(-1, fifo.erase_latest(1));
    EXPECT_EQ(0, fifo.size());

    // case 3: one node
    fifo.push_back(1);
    EXPECT_EQ(fifo.size(), 1);
    EXPECT_EQ(0, fifo.erase_latest(1));
    EXPECT_EQ(fifo.size(), 0);
    EXPECT_EQ(-1, fifo.pop(top));

    // case 4: multi node delete head
    fifo.push_back(1); // head will be deleted
    fifo.push_back(2);
    EXPECT_EQ(2, fifo.size());
    EXPECT_EQ(0, fifo.erase_latest(1));
    EXPECT_EQ(1, fifo.size());
    EXPECT_EQ(0, fifo.pop(top));
    EXPECT_EQ(2, top);
    EXPECT_EQ(0, fifo.size());

    // case 5: multi node delete tail
    fifo.push_back(1); 
    fifo.push_back(2); // tail will be deleted
    EXPECT_EQ(2, fifo.size());
    EXPECT_EQ(0, fifo.erase_latest(2));
    EXPECT_EQ(1, fifo.size());
    EXPECT_EQ(0, fifo.pop(top));
    EXPECT_EQ(1, top);
    EXPECT_EQ(0, fifo.size());

    // case 6: multi node delete middle
    fifo.push_back(1); 
    fifo.push_back(2); // middle will be deleted
    fifo.push_back(3); 
    EXPECT_EQ(3, fifo.size());
    EXPECT_EQ(0, fifo.erase_latest(2));
    EXPECT_EQ(2, fifo.size());
    EXPECT_EQ(0, fifo.pop(top));
    EXPECT_EQ(1, top);
    EXPECT_EQ(0, fifo.pop(top));
    EXPECT_EQ(3, top);
    EXPECT_EQ(0, fifo.size());

    // case 7: have same value but delete newest first
    // 1 2 1 3  -> erase(1) -> 2 1 3
    fifo.push_back(1);
    fifo.push_back(2);
    fifo.push_back(1);
    fifo.push_back(3);
    EXPECT_EQ(4, fifo.size());
    EXPECT_EQ(0, fifo.erase_latest(1));
    EXPECT_EQ(3, fifo.size());
    EXPECT_EQ(0, fifo.pop(top));
    EXPECT_EQ(2, top);
    EXPECT_EQ(0, fifo.pop(top));
    EXPECT_EQ(1, top);
    EXPECT_EQ(0, fifo.pop(top));
    EXPECT_EQ(3, top);
    EXPECT_EQ(0, fifo.size());
}
