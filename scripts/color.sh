# Init option {{{
Color_off='\033[0m'       # Text Reset

# terminal color template {{{
# Regular Colors
Black='\033[0;30m'        # Black
Red='\033[0;31m'          # Red
Green='\033[0;32m'        # Green
Yellow='\033[0;33m'       # Yellow
Blue='\033[0;34m'         # Blue
Purple='\033[0;35m'       # Purple
Cyan='\033[0;36m'         # Cyan
White='\033[0;37m'        # White

# Bold
BBlack='\033[1;30m'       # Black
BRed='\033[1;31m'         # Red
BGreen='\033[1;32m'       # Green
BYellow='\033[1;33m'      # Yellow
BBlue='\033[1;34m'        # Blue
BPurple='\033[1;35m'      # Purple
BCyan='\033[1;36m'        # Cyan
BWhite='\033[1;37m'       # White

# }}}
# 打印函数
function echo_red() {
  echo -e "${Red}$1${Color_off}"
}

function echo_green() {
  echo -e "${Green}$1${Color_off}"
}

function echo_yellow() {
  echo -e "${Yellow}$1${Color_off}"
}

function echo_blue() {
  echo -e "${Blue}$1${Color_off}"
}

function echo_white() {
  echo -e "${White}$1${Color_off}"
}

function echo_default() {
  echo -e "$1"
}

# 带标签的打印函数
function print_info() {
  echo -e "${Green}[INFO]${Color_off} $1"
}

function print_warn() {
  echo -e "${Yellow}[WARN]${Color_off} $1"
}

function print_error() {
  echo -e "${Red}[ERROR]${Color_off} $1"
}

function print_success() {
  echo -e "${BGreen}[SUCCESS]${Color_off} $1"
}

function print_debug() {
  echo -e "${Cyan}[DEBUG]${Color_off} $1"
}
