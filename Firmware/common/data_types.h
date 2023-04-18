#include "driver/twai.h"
#include <stdbool.h>

#define MAX_FRAMES 16


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

