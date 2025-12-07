use mimalloc::MiMalloc;
use rand::{SeedableRng, rngs::SmallRng, seq::IndexedRandom};
use std::{hint::black_box, sync::atomic::AtomicUsize, thread, time::Instant};

// c.f. https://github.com/microsoft/mimalloc/issues/1104

#[global_allocator]
static GLOBAL: MiMalloc = MiMalloc;

const LENGTHS: [usize; 4] = [1000, 5000, 10000, 50000];

fn main() {
    let start = Instant::now();
    let gsum = AtomicUsize::new(0);

    thread::scope(|scope| {
        for _ in 0..24 {
            scope.spawn(|| {
                let mut rng = SmallRng::from_os_rng();
                let mut sum = 0;
                for _ in 0..500_000 {
                    let len = LENGTHS.choose(&mut rng).unwrap();
                    let mut allocation = black_box(vec![0i32; *len]);
                    allocation[0] = 1;
                    sum += allocation.choose(&mut rng).unwrap();
                }
                print!(".");
                gsum.fetch_add(sum as usize, std::sync::atomic::Ordering::Relaxed);
            });
        }
    });
    println!();

    let duration = start.elapsed();
    println!("took: {}", duration.as_millis());
    println!("gsum: {}", gsum.load(std::sync::atomic::Ordering::Relaxed));
}
