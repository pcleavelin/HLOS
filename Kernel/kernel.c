extern void RESET_VIDEO_MODE();
extern void PRINT_STRING();
extern char *HELLO_C_MSG;

void main() {
    RESET_VIDEO_MODE();
    PRINT_STRING(hello_c_msg);
}