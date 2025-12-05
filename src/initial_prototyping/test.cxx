#include <thread>
#include <iostream>
using namespace std;
#include <ncurses.h>

void task1(){
    cout<<"Thread1:"<< this_thread::get_id() << "\n";
}

void task2(){
    cout<<"Thread2:"<< this_thread::get_id() << "\n";
}

void task3(){
    cout<<"Thread3:"<< this_thread::get_id() << "\n";
}

int main(){
    thread t1(task1);
    thread t2(task2);
    thread t3(task3);

    // Get thread IDs
    cout << "t1 ID: " << t1.get_id() << "\n";
    cout << "t2 ID: " << t2.get_id() << "\n";

    // Join t1 if joinable
    if (t1.joinable()) {
        t1.join();
        cout << "t1 joined\n";
        cout << t1.get_id() << "\n";
    }

    // Detach t2
    if (t2.joinable()) {
        t2.detach();
        cout << "t2 detached\n";
    }

    cout << "Main thread sleeping for 1 second...\n";
    this_thread::sleep_for(chrono::seconds(1));
    cout << "Main thread awake.\n";

    return 0;
}

