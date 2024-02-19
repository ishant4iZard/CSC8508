#ifndef _FMOD_PS4_H
#define _FMOD_PS4_H

#include <user_service.h>
#include "fmod_common.h"

/*
[ENUM]
[
    [DESCRIPTION]
    Cores available for thread assignment.

    [REMARKS]

    [SEE_ALSO]
    FMOD_PS4_THREADAFFINITY
]
*/
typedef unsigned int FMOD_THREAD;

#define FMOD_THREAD_DEFAULT                0         /* Use default thread assignment. */
#define FMOD_THREAD_CORE0                  (1 << 0)
#define FMOD_THREAD_CORE1                  (1 << 1)
#define FMOD_THREAD_CORE2                  (1 << 2)  /* Default for all threads. */
#define FMOD_THREAD_CORE3                  (1 << 3)
#define FMOD_THREAD_CORE4                  (1 << 4)
#define FMOD_THREAD_CORE5                  (1 << 5)
#define FMOD_THREAD_CORE6                  (1 << 6)


/*
[STRUCTURE]
[
    [DESCRIPTION]
    Mapping of cores to threads.

    [REMARKS]

    [SEE_ALSO]
    FMOD_THREAD
    FMOD_PS4_SetThreadAffinity
]
*/
typedef struct FMOD_PS4_THREADAFFINITY
{
    FMOD_THREAD mixer;             /* Software mixer thread. */
    FMOD_THREAD feeder;            /* Audio hardware feeder thread. */
    FMOD_THREAD stream;            /* Stream thread. */
    FMOD_THREAD nonblocking;       /* Asynchronous sound loading thread. */
    FMOD_THREAD file;              /* File thread. */
    FMOD_THREAD geometry;          /* Geometry processing thread. */
    FMOD_THREAD recording;         /* Audio input feeder thread. */
    FMOD_THREAD profiler;          /* Profiler threads. */
    FMOD_THREAD studioUpdate;      /* Studio update thread. */
    FMOD_THREAD studioLoadBank;    /* Studio bank loading thread. */
    FMOD_THREAD studioLoadSample;  /* Studio sample loading thread. */
} FMOD_PS4_THREADAFFINITY;


/*
[API]
[
    [DESCRIPTION]
    Control which core particular FMOD threads are created on.

    [PARAMETERS]
    'affinity'    Pointer to a structure that describes the affinity for each thread.

    [REMARKS]
    Call before System::init or affinity values will not apply.

    [SEE_ALSO]
    FMOD_PS4_THREADAFFINITY
]
*/
extern "C" FMOD_RESULT F_API FMOD_PS4_SetThreadAffinity(FMOD_PS4_THREADAFFINITY *affinity);


/*
[ENUM]
[
    [DESCRIPTION]
    Port types avaliable for routing audio.

    [REMARKS]

    [SEE_ALSO]
    System::AttachChannelGroupToPort
    FMOD_PORT_TYPE
]
*/
typedef enum FMOD_PS4_PORT_TYPE
{
    FMOD_PS4_PORT_TYPE_MUSIC,           /* Background music, pass FMOD_PORT_INDEX_NONE as port index */
    FMOD_PS4_PORT_TYPE_VOICE,           /* Voice chat, pass SceUserServiceUserId of desired user as port index */
    FMOD_PS4_PORT_TYPE_PERSONAL,        /* Personal audio device, pass SceUserServiceUserId of desired user as port index */
    FMOD_PS4_PORT_TYPE_CONTROLLER,      /* Controller speaker, pass SceUserServiceUserId of desired user as port index */
    FMOD_PS4_PORT_TYPE_COPYRIGHT_MUSIC, /* Copyright background music, pass FMOD_PORT_INDEX_NONE as port index. You cannot have both copyright and non-copyright music open simultaneously */
    FMOD_PS4_PORT_TYPE_SOCIAL,          /* VR only - social screen when in separate mode otherwise not output, pass FMOD_PORT_INDEX_NONE as port index. */
    FMOD_PS4_PORT_TYPE_MAX              /* Maximum number of port types supported. */
} FMOD_PS4_PORT_TYPE;


/*
[API]
[
    [DESCRIPTION]
    Retrieve the output volume of the pad speaker as set by the user in the system software.

    [PARAMETERS]
    'userID'  SceUserServiceUserId that identifies a logged in user.
    'volume'  Output parameter to receive the volume

    [REMARKS]
    There must be at least one channel group attached to the users pad speaker for this function to work.

    [SEE_ALSO]
    System::AttachChannelGroupToPort
]
*/
extern "C" FMOD_RESULT F_API FMOD_PS4_GetPadVolume(FMOD_SYSTEM* system, SceUserServiceUserId userID, float* volume);


/*
[API]
[
    [DESCRIPTION]
    Allocate GPU compute resources to enable some processing to be offloaded to the GPU.

    [PARAMETERS]
    'computePipe'   TBA
    'computeQueue'  TBA
    'garlicMem'     TBA
    'garlicMemSize' TBA
    'onionMem'      TBA
    'onionMemSize'  TBA

    [REMARKS]

    [SEE_ALSO]
    FMOD_PS4_ReleaseComputeDevice
]
*/
extern "C" FMOD_RESULT F_API FMOD_PS4_SetComputeDevice(int computePipe, int computeQueue, void* garlicMem, size_t garlicMemSize, void* onionMem, size_t onionMemSize);


/*
[API]
[
    [DESCRIPTION]
    Release all GPU compute resources and no longer offload processing.

    [PARAMETERS]

    [REMARKS]

    [SEE_ALSO]
    FMOD_PS4_SetComputeDevice
]
*/
extern "C" FMOD_RESULT F_API FMOD_PS4_ReleaseComputeDevice();

#endif /* _FMOD_PS4_H */
