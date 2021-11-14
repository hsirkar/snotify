# snotify
Get text messages/email alerts when your Deepthought2 jobs finish running.

## Getting started

Somewhere in your Deepthought2 space, clone this repository, build the C program, and add it to PATH:

    bash
    git clone https://www.github.com/hsirkar/snotify
    cd snotify
    make
    export PATH=$PATH:$(pwd)

## Usage

First, run your submission script.

    > sbatch submit.sh
    Submitted batch job 12345678

To get an email when job 12345678 completes:

    snotify -j 12345678 -e test@example.com

To get a text message:

    snotify -j 12345678 -p 3285550156 -c verizon

Supported carriers (not all tested):

- `att`
- `sprint`
- `tmobile`
- `verizon`
- `boost`
- `cricket`
- `ting`
- `virgin`
- `uscellular`
- `straighttalk`
- `metropcs`

## Testing
If you want to verify that snotify works for your email or phone, you can test with the provided `hello-world` program:

    cd test
    make
    sbatch submit.sh
    snotify -j <job-id> -e <email> | -p <phone> -c <carrier>

## How it works

snotify runs in the background and listens for changes to squeue. If it sees that your job id is no longer in the queue, it sends you an email/text notification saying your job is complete, and terminates.

Most cell carriers provide a gateway that lets you text a number by sending an email. This is what snotify uses to send text messages, and this is why the carrier needs to be known.

## Contributing
I literally wrote this in one day, so I'm sure it's full of bugs. If you can fix something or want to add a feature, then feel free to send a PR. Contributions are absolutely welcome!

## Disclaimer
The code is still in alpha and not everything has been tested. If this program causes anything to break or go wrong, then I'm not responsible for anything. If you need to terminate it, use `ps -xj | grep snotify` to get the process id, and then `pkill` it.

Good luck with your projects!
