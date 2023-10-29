use std::error;
use std::fs::{File, read_dir, read_to_string};
use std::io::{self, BufRead, BufReader, Read};
use std::collections::HashMap;
use std::time::Instant;
use regex::Regex;


use canparse::pgn::{PgnLibrary, SpnDefinition, ParseMessage};

use can_dbc::{DBC, ByteOrder, Signal, MultiplexIndicator};


const DIRECTORY: &str = "../../CAN_logs";

const EFF_FLAG: u32 = 0x8000_0000;

struct CANFrame {
    timestamp: f64,
    id: u32,
    dlc: u8,
    data: Vec<u8>,
}

fn main() {
    let dbc_now = Instant::now();
    
    let mut f = File::open("../../../../WiCAN_GUI/resources/2018CAR.dbc").unwrap();
    let mut buffer = Vec::new();
    f.read_to_end(&mut buffer).unwrap();

    let dbc = DBC::from_slice(&buffer).expect("Failed to parse dbc file");

    let mut signal_lookup = HashMap::new();

    for msg in dbc.messages() {
        signal_lookup.insert(
            msg.message_id().0 & !EFF_FLAG,  // TODO: See if we need this EFF flag here, something with extended frames for sure
            (msg.message_name().clone(), msg.signals().clone(), "memo"),
        );
    }

    println!("Took {}s to parse DBC", dbc_now.elapsed().as_secs_f32());



    /*  iterate through all messaged in DBC */
    // for message in dbc.messages() {
    //     println!("{:#?} {:#?}", message.message_name(), message.message_id());
    //     if message.message_name() == "BMU_CellVoltage"{
    //         for signal in message.signals(){
    //             println!("{:#?}", signal.name())
    //         }
    //         break;
    //     }
    //     // for signal in message.signals(){
    //     //     println!("{:#?}", signal);
    //     // }
    // }


    // Setup regex
    let re = Regex::new( r"^\s*?\((?P<timestamp>[\d.]+)\)\s+(?P<channel>[a-zA-Z0-9]+)\s+(?P<can_id>[0-9A-F]+)\s+\[\d+\]\s*(?P<can_data>[0-9A-F ]*).*?$").unwrap();
    // let now = Instant::now();
    let paths = read_dir(DIRECTORY).unwrap();
    let mut total_lines = 0;
    let mut errors = 0;

    let now = Instant::now();
    let temp_file = File::open("test_log.log").unwrap();

    for path in paths {
        let file_now = Instant::now();
        let file = File::open(path.unwrap().path()).unwrap();
        // let file = temp_file;
        // println!("Path: {:?}", fp.clone().into_os_string().into_string().unwrap());
        // println!("Opened File");

        // let reader = BufReader::new(File::open("test_log.log").unwrap());
        let reader = BufReader::new(file);

        // best shot at actually getting this to work is here:
        // https://github.com/rhinocerose/canviz/blob/master/src/main.rs

        for line in reader.lines(){
            total_lines += 1;

            if let Some(caps) = re.captures(line.as_ref().unwrap()) {
                // println!("Regex Matched");
                // println!("Timestamp: {} Channel: {} CAN ID: {} CAN Data: {}", &caps["timestamp"], &caps["channel"], &caps["can_id"], &caps["can_data"]);
                let data_str = &caps["can_data"];
                let parts: Vec<u8> = data_str.split_whitespace().map(|x| u8::from_str_radix(x, 16).unwrap()).collect();


                let frame = CANFrame {
                    timestamp: caps["timestamp"].parse::<f64>().unwrap(),
                    id: u32::from_str_radix(&caps["can_id"], 16).expect("Failed to parse CAN ID"),
                    data: parts.clone(),
                    dlc: parts.clone().len() as u8,
                };

                // println!("CAN Frame Struct: {:#?}", frame);

                if frame.dlc > 0{
                    if !print_dbc_signals(&signal_lookup, &frame, true){
                        errors += 1;
                    }
                }
                else{
                    errors += 1;
                }

            }
            else {
                // print!("No match!:  ");
                // println!("{}", line.as_ref().unwrap());
                errors += 1;
            }
        }
        // break;
        // println!("Took {} seconds to parse file", file_now.elapsed().as_secs_f32());
    }
    println!("Took {} seconds to parse {} lines with {} errors", now.elapsed().as_secs_f32(), total_lines, errors);
}

// Given a CAN Frame, lookup the can signals and print the signal values
// From: https://github.com/rhinocerose/canviz/blob/master/src/main.rs
fn print_dbc_signals (signal_lookup: &HashMap<u32, (String, Vec<Signal>, &str)>,
                      frame: &CANFrame,
                      raw_data: bool, ) -> bool {

    let debug_prints = false;

    let id = frame.id & !EFF_FLAG;
    if let Some(entry) = signal_lookup.get(&id) { // Handle unknown ids
        let message_name = entry.0.clone();
        let signals = entry.1.clone();
        let _comment = entry.2;

        let message_name_s = format!("{:<30}", message_name);
        if debug_prints {println!("\n{} Frame ID: {:08X}", message_name_s, frame.id);}
        // println!("\n{} Frame ID: {}", message_name_s, frame.id);

        for signal in signals.iter() {
            let mut frame_data: [u8; 8];
            match frame.data.clone().try_into() {
                Ok(data) => {
                    frame_data = data;
                    let signal_value: u64 = if *signal.byte_order() == ByteOrder::LittleEndian {
                        u64::from_le_bytes(frame_data)
                    } else {
                        u64::from_be_bytes(frame_data)
                    };
            
                    // Calculate signal value
                    let bit_mask: u64 = 2u64.pow(*signal.signal_size() as u32) - 1;
                    let signal_value = ((signal_value >> signal.start_bit()) & bit_mask) as f32
                                        * *signal.factor() as f32
                                        + *signal.offset() as f32;
            
                    let signal_value_s = format!("{:6.2}", signal_value);
                    let signal_name_s = format!("{:<30}", signal.name());
            
                    let muxed: bool = *signal.multiplexer_indicator() == MultiplexIndicator::MultiplexedSignal(frame_data[0] as u64);
                    let plain: bool = *signal.multiplexer_indicator() == MultiplexIndicator::Plain;
            
                    if (plain || (raw_data && muxed)) && debug_prints{

                        println!(
                            "{} → {} {}", signal_name_s, signal_value_s.clone(), signal.unit()
                        );
                    };
                }
                Err(_) => {
                    // println!("Error parsing data");
                    return false;
                }
            }
    
            
        }
    }
    else {
        // println!("Unknown Message ID");
        return false;
    }

    return true;
    
}