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