use std::env;
use std::io::{self, Write};
use std::process::Command;

fn main() {
    loop {
        match env::current_dir() {
            Ok(path) => print!("{}> ", path.display()),
            Err(_) => print!("> "),
        }
        io::stdout().flush().unwrap();

        let mut input = String::new();
        if io::stdin().read_line(&mut input).is_err() {
            break;
        }

        let cmd = input.trim();
        if cmd.is_empty() {
            continue;
        }

        if cmd.eq_ignore_ascii_case("exit") {
            break;
        }

        if cmd.eq_ignore_ascii_case("cd") {
            if let Some(home) = env::var_os("USERPROFILE") {
                if let Err(e) = env::set_current_dir(home) {
                    eprintln!("The system cannot find the path specified: {e}");
                }
            }
            continue;
        } else if let Some(rest) = cmd.strip_prefix("cd ").or_else(|| cmd.strip_prefix("CD ")) {
            let target = rest.trim().trim_matches('"');
            if let Err(e) = env::set_current_dir(target) {
                eprintln!("The system cannot find the path specified: {e}");
            }
            continue;
        }

        let status = Command::new("cmd")
            .args(["/C", cmd])
            .status();

        if let Err(e) = status {
            eprintln!("Failed To Execute: {e}");
        }
    }
}