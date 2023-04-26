use std::io::{self, Write, BufRead, BufReader};
use std::ops::Index;
use std::time::{Duration, Instant};


// Defines

// maximum rx buffer len: extended CAN frame with timestamp
const SLCAN_MTU: usize = "T1111222281122334455667788EA5F\r".len() + 1;
const SLCAN_CMD_LEN: usize = 1;
const SLCAN_EXT_ID_LEN: usize = 8;

const BELL: u8 = 0x07;
const CARRIAGE_RETURN: u8 = '\r' as u8;
const TRANSMIT_COMMAND: u8 = 'T' as u8;

pub struct CanFrame {
    pub id: usize,
    pub dlc: u8,
    pub data: [u8; 8],
}

impl std::fmt::Display for CanFrame {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "ID: {}\nDLC: {}\nData: {:?}\n", self.id, self.dlc, self.data)
    }
}



fn main() {
    let mut port = serialport::new("COM12", 115_200)
        .timeout(Duration::from_micros(1000))
        .open()
        // expect does exactly the same thing as your match did
        .expect("Error: ");

    // in your python you use readline, to archive this
    // in rust the easiest way is to use a BufReader
    let mut port = BufReader::new(port);
    let mut line_buffer = String::new();
    let mut buf = vec![];

    let mut start_time = Instant::now();

    let mut counter = 0;


    loop {

        // clear the line buffer to use it again
        buf.clear();
        // previously you never stopped reading
        match port.read_until('\r' as u8 , &mut buf){
            Ok(_) =>{
                let s = match std::str::from_utf8(&buf) {
                    Ok(v) => v,
                    Err(e) => panic!("Invalid UTF-8 sequence: {}", e),
                };
                let result = ser_to_can(&s);

                match result {
                    Ok(can_msg) => {
                        counter += 1;
                        // println!("{}", can_msg);
                        // Send to ZMQ here
                    },
                    Err(error) => println!("Error"),
                }
            },
            Err(_) => (),
        }

        if (start_time.elapsed().as_millis()) > 1000 {
            println!("{:.1$} msgs/s", counter as f64/(start_time.elapsed().as_millis() as f64/1000.0), 2);
            counter = 0;
            start_time = Instant::now();
        }
    }
}

// Convert SLCAN serial data to CAN Frame
fn ser_to_can(input_string: &str) -> Result<CanFrame, String>{

    let mut can_msg = CanFrame {id: 0, dlc: 0, data: [0; 8]};

    let binding = input_string.to_string();
    let mut slcan_string = binding.as_bytes();



    match slcan_string[0] as char {
        'T' => {
            let mut msg_position: usize = 1;
            while msg_position <= SLCAN_EXT_ID_LEN {
                can_msg.id *= 16;
                can_msg.id += char_to_int(slcan_string[msg_position]) as usize;
                msg_position += 1;
            }
            // println!("ID: {}", can_msg.id);

            can_msg.dlc = char_to_int(slcan_string[msg_position]);
            msg_position += 1;

            // println!("DLC: {}", can_msg.dlc);

            // print!("Data: ");

            for i in 0..can_msg.dlc as usize {
                can_msg.data[i] = (char_to_int(slcan_string[msg_position]) << 4) + char_to_int(slcan_string[msg_position+1]);
                msg_position += 2;

                // print!("{} ", can_msg.data[i]);
            }
            // print!("\n");

        },
        _ => return Err("Not Extended Frame".to_string()),
    }
    // println!("{}", slcan_string[0] as char);

    Ok(can_msg)

}


// Convert CAN Frame into SLCAN representation
fn can_to_ser(can_msg: CanFrame){

}

fn char_to_int(in_char: u8) -> u8{
    return in_char - '0' as u8;
}