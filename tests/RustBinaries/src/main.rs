//use std::env;
use std::fs;

fn main() {
    println!("File test: open");
	
	let filename = "/data/local/tmp/testfile";

	let contents = fs::read_to_string(filename)
        .expect("Something went wrong reading file");

    println!("With text: {}", contents);
}
