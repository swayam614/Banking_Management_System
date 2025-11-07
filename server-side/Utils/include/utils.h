#ifndef __UTILS_H__
#define __UTILS_H__

int read_line(int fd, char *buffer);
int lock_file(int fd, int lock_type);
void unlock_file(int fd);
int authenticate_user(char *, char *, int);
int change_password_user(const char *, const char *, int);
#endif
