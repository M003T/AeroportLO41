#ifndef IPC_H
#define IPC_H

#define PlaneCreateNewDelay 7 //Default 7, has randompart

#define PlaneGenerateBigOdds 3 //Default 3, means 1 out of 3 is big

#define PlaneTakeOfOrLandingDelay 20 // Default 20, has randompart

#define PlaneFuelLvlRange 30 //Default 30
#define PlaneFuelInsuredThreshold 15 //Default 15
#define PlaneFuelUrgentThreshold 5 //Default 5

#define PlaneLoseFuelDelay 10 //Default 10

#define ControllBarrierOdds 25 //Default 25
#define ControllBarrierDelay 20 //Default 20, has randompart

#define ControllBarrierDelayReduceWhenInsuredPlane 10 //Default 10, means when Planes become assured, if there is Barrier then its delay is reduced by 10

#define ControllNumberBeforeSmallOnTrack1 3 //Default 3, means when there are 3 planes more awaiting on track 2 than on track 1, small planes go on track 1

#endif