#pragma once

#include "wled.h"

#ifndef WATCHDOG_DELAY_INTERVAL
#define WATCHDOG_DELAY_INTERVAL 250
#endif

#ifndef BACKGROUND_TASK_SIZE
#define BACKGROUND_TASK_SIZE 10000
#endif

class UsermodMultiThread : public Usermod {

  private:
    TaskHandle_t BackgroundTaskHandle;

  public:

    // backgroundTask() is the container that calls backgroundLoop repeatedly. 
    static void backgroundTask(void *parameter)
    {
      UsermodMultiThread *instance = static_cast<UsermodMultiThread *>(parameter);
      
      for (;;)
      {
        instance->backgroundLoop();
        EVERY_N_MILLISECONDS( WATCHDOG_DELAY_INTERVAL ){ delay(1); }
      }
    }

    void initBackground(){
      xTaskCreatePinnedToCore(
          backgroundTask,       /* Task function. */
          "BGTask",             /* name of task. */
          BACKGROUND_TASK_SIZE, /* Stack size of task */
          this,                 /* parameter of the task */
          1,                    /* priority of the task */
          &BackgroundTaskHandle,         /* Task handle to keep track of created task */
          xPortGetCoreID() == 0 ? 1 : 0); /* pin task to other core */      
    }

    void setup() {
      Serial.println("Multi Thread Start");
      // Do your normal setup here
      
      // Initializing the background task, do this last
      initBackground();
    }

    // The background loop
    virtual void backgroundLoop(){
      // Implement your background thread operations here.
      // EVERY_N_MILLISECONDS( 1000 ){ doSomethingCool(); }
      // EVERY_N_MILLISECONDS( 1500 ){ doAnotherCoolThing(); }
      // EVERY_N_MILLISECONDS( 5500 ){ tellYourMomYouLoveHer(); }
    }

    // The main loop
    void loop() {
      // Implement your main thread operations here.
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID
     */
    uint16_t getId() {
      return USERMOD_ID_MULTI_THREAD;
    }

};