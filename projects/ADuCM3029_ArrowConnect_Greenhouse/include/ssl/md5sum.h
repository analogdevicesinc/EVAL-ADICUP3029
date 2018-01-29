#if !defined(ARROW_MD5SUM_H_)
#define ARROW_MD5SUM_H_

int md5sum(char *hash, const char *data, int len);

void md5_chunk_init();
void md5_chunk(const char *data, int len);
void md5_chunk_hash(char *hash);

#endif  // ARROW_MD5SUM_H_
