/**
 * strstr(str, target): Search if 'str' contains 'target' -> move pointer to the first char of the 'target'
 * strncmp(str1, str2, byte_num): compare first 'byte_num' bytes of the 'str1' to 'str2'
 * strrchr(str, c): searches for the last occurrence of the character 'c' in the 'str' pointed to
 * scandir(): Scanf through all directories inside the ... folder
 * gethostbyname(domain): Get host by name
 * setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)); : Set broadcast option
 */

// Gửi đến khi nào hết nội dung (đề phòng trường hợp lỗi)
int SendPacket(int fd, char *data, int len)
{
    int sent = 0;
    do
    {
        sent += send(fd, data + sent, len - sent, 0);
    } while (sent >= 0 && sent < len);
    return sent;
}

// Nhận đến khi nào hết nội dung (đề phòng trường hợp lỗi)
int RecvPacket(int fd, char *data, int maxlen)
{
    int received = 0;
    int block_size = 2;
    int tmp = 0;
    do
    {
        tmp = recv(fd, data + received, block_size, 0);
        received += tmp;
    } while (received >= 0 && received < maxlen && tmp == block_size);
    return received;
}

// Hàm nối 1 xâu vào 1 xâu khác
void Append(char **pdst, const char *src)
{
    // Xâu ký tự cũ
    char *dst = *pdst;
    // Tính độ dài ban đầu của xâu
    int oldLen = (dst == NULL ? 0 : strlen(dst));
    // Độ dài mới
    int newLen = oldLen + strlen(src) + 1; // Note: Đối với xâu nên cấp phát thêm 1 byte (làm byte lính canh ở cuối)
    dst = (char *)realloc(dst, newLen);
    // Set tất cả bytes từ cuối xâu cũ = 0 (Set các bytes mới cấp phát = 0)
    memset(dst + oldLen, 0, strlen(src) + 1);
    // Nối xâu vào cuối xâu ban đầu
    sprintf(dst + oldLen, "%s", src);

    // Dưa lại giá trị dst vào pdst
    *pdst = dst;
}

// So sánh 2 cấu trúc thư mục (là kiểu directory entries)
int Compare(const struct dirent **a, const struct dirent **b)
{
    if ((*a)->d_type == (*b)->d_type)
        return 0;
    else if ((*a)->d_type == DT_DIR)
        return -1;
    else
        return 1;
}

// Lấy đuôi của file name (sau dấu ".")
const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}

void sighandler(int signum)
{
    int stat = 0;
    // Kill zombie processes
    while (waitpid(-1, &stat, WNOHANG) > 0)
        ;
}