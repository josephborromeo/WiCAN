use std::fs::{File, read_dir, read_to_string};
use std::io::{self, BufRead};
use std::path::Path;
use std::time::Instant;

const DIRECTORY: &str = "../../CAN_logs";

fn main() {
    let now = Instant::now();
    let paths = read_dir(DIRECTORY).unwrap();
    let mut total_lines = 0;

    for path in paths {
        // let fp = path.unwrap().path();
        // // File hosts.txt must exist in the current path
        // let contents = read_to_string(fp)
        // .expect("Should have been able to read the file");

        // NEED to parse and decode files using DBC
        
        
        if let Ok(lines) = read_lines(path.unwrap().path()) {
            // Consumes the iterator, returns an (Optional) String
            for line in lines {
                if let Ok(ip) = line {
                    // println!("{}", ip);
                    total_lines += 1;
                }
            }
        }
    }
    
    println!("Took {} seconds to parse {} lines", now.elapsed().as_millis() as f64/1000.0, total_lines);
}

// The output is wrapped in a Result to allow matching on errors
// Returns an Iterator to the Reader of the lines of the file.
fn read_lines<P>(filename: P) -> io::Result<io::Lines<io::BufReader<File>>>
where P: AsRef<Path>, {
    let file = File::open(filename)?;
    Ok(io::BufReader::new(file).lines())
}