#include "driver/twai.h"
#include <stdbool.h>


/*  ESP-NOW Max Transfer size is 250 bytes 
    CAN_frame_t is ~14 bytes (maybe up to 17 depending on bool size)
    Package size is N*CAN_frame_t size + 1 byte for num_frames

    Therefore conservatively: MAX_FRAMES = (250-1)/17 = 14.6 = 14 
    Theoretical max: MAX_FRAMES = (250-1)/14 = 17.8 = 17 
*/ 
#define MAX_FRAMES 14

// MIN macro
#define MIN(a,b) (((a)<(b))?(a):(b))

// Slimmed down CAN Frame
// 49 bits? -- Check Bool Size 
typedef struct __attribute__((__packed__)){
    uint32_t identifier;                    /**< 11 or 29 bit identifier */
    bool extd : 1;                          /**< Extended Frame Format (29bit ID) */
    uint8_t data_length_code;               /**< Data length code */
    uint8_t data[TWAI_FRAME_MAX_DLC];       /**< Data bytes (not relevant in RTR frame) */
} CAN_frame_t;

// Custom packet datastructure
typedef struct{
    uint8_t num_frames;                     /* Number of Frames being sent */
    CAN_frame_t frames[MAX_FRAMES];         // Add timestamp in
} wican_data_t;

