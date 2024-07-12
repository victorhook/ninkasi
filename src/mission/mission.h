#ifndef MISSION_H
#define MISSION_H


/*
    Mission

*/

class Mission
{
    public:
        static Mission* get_startup_mission();
        void start();
        void stop();

    protected:
        Mission();
        ~Mission();
        void setup();
        void run();
        void teardown();

};


#endif /* MISSION_H */
