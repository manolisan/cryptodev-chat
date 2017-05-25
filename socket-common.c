#include "socket-common.h"

/* Insist until all of the data has been written */
ssize_t insist_write(int fd, const void *buf, size_t cnt)
{
	ssize_t ret;
	size_t orig_cnt = cnt;

	while (cnt > 0) {
		ret = write(fd, buf, cnt);
		if (ret < 0)
		return ret;
		buf += ret;
		cnt -= ret;
	}
	return orig_cnt;
}

ssize_t insist_read(int fd, void *buf, size_t cnt)
{
	ssize_t ret;
	size_t orig_cnt = cnt;

	while (cnt > 0) {
		ret = read(fd, buf, cnt);
		if (ret < 0){
			return ret;
		}
		buf += ret;
		cnt -= ret;
	}
	return orig_cnt;
}

int get_and_print(char * buf, int sd){
	/// sizeof buf??? when o put the change of line?
	ssize_t n;
	uint32_t size;

	if(encrypted==1){
		n = insist_read(sd, buf, DATA_SIZE);
		if (n < 0) {
			perror("read");
			exit(1);
		}
		if (n <= 0) return 1;

		decrypt(buf);

		memcpy(&size, buf, sizeof(size));
		size = ntohl(size);
		//printf("Size %ld->", size);

		buf += sizeof(size);

	}
	else{
		n  = insist_read(sd, &size, sizeof(size));
		if (n < 0) {
			perror("read");
			exit(1);
		}
		if (n <= 0) return 1;

		size = ntohl(size);
		n = insist_read(sd, buf, size);
		if (n < 0) {
			perror("read");
			exit(1);
		}
		if (n <= 0) return 1;
	}



	int saved_point = rl_point;
	char *saved_line = rl_copy_text(0, rl_end);
	rl_save_prompt();
	rl_replace_line("", 0);
	rl_redisplay();


	// Write message to stdout
	if (insist_write(0, buf, size) != size) {
		perror("write");
		exit(1);
	}
	//printf("\n");


	rl_restore_prompt();
	rl_replace_line(saved_line, 0);
	rl_point = saved_point;
	rl_redisplay();
	free(saved_line);

	return 0;
}

void my_rlhandler(char* line)
{
	if (line == NULL) return; //???? to change!!!!!

	size_t len = strlen(line);
	size_t name_len = strlen(prompt);
	if (*line != 0) {
		add_history(line);

		// send the length of the message
		uint32_t wlen = htonl(len + name_len + 1);

		/*
		//encrypt(&wlen, sizeof(wlen));
		if( insist_write(newsd, &wlen, sizeof(wlen)) != sizeof(wlen)){
			perror("write");
			exit(1);
		}
		*/

		// format messasge
		//char * full_message = malloc(len + name_len + 1); // 1 for a space, 1 for line changing and 1 for a \0
		char * full_message = malloc(BUFF_SIZE);
		memcpy(full_message, &wlen, sizeof(wlen));
		strcpy(full_message + sizeof(wlen) , prompt);
		strcpy(full_message + name_len  + sizeof(wlen), line);
		full_message[len +name_len  + sizeof(wlen)] = '\n';

		if (encrypted==1){
			encrypt(full_message, len+name_len + sizeof(wlen) + 1);
			if(insist_write(newsd, full_message, DATA_SIZE) != DATA_SIZE){
				perror("write");
				exit(1);
			}
		}
		else{
			if(insist_write(newsd, full_message, len+name_len+1) != len+name_len+1){
				perror("write");
				exit(1);
			}
		}
		free(full_message);
	}
	free (line);
}

void encrypt(void *buffer, size_t enc_s){

	int i;
	struct session_op sess;
	struct crypt_op cryp;
	struct {
		unsigned char 	in[DATA_SIZE],
		encrypted[DATA_SIZE],
		decrypted[DATA_SIZE],
		iv[BLOCK_SIZE],
		key[KEY_SIZE];
	} data;

	memset(&sess, 0, sizeof(sess));
	memset(&cryp, 0, sizeof(cryp));

	int cfd = open("/dev/crypto", O_RDWR);
	if (cfd < 0) {
		perror("open(/dev/crypto)");
		exit(1);
	}

	//Initialize encyption key and initialization vector
	for(i=0; i<BLOCK_SIZE; i++){
		data.iv[i]='1';
		data.key[i]='0'+i;
	}

	sess.cipher = CRYPTO_AES_CBC;
	sess.keylen = KEY_SIZE;
	sess.key = data.key;

	if (ioctl(cfd, CIOCGSESSION, &sess)) {
		perror("ioctl(CIOCGSESSION)");
		exit(1);
	}

	/*
	* Encrypt data.in to data.encrypted
	*/
	cryp.ses = sess.ses;
	//cryp.len = enc_s;
	cryp.len = sizeof(data.in);
	memcpy(data.in, buffer, enc_s);
	for(i=enc_s; i<DATA_SIZE; i++){
		data.in[i]='0';
	}
	cryp.src = data.in;
	cryp.dst = data.encrypted;
	cryp.iv = data.iv;
	cryp.op = COP_ENCRYPT;

	/*printf("\nOriginal data:\n");
	for (i = 0; i < DATA_SIZE; i++)
	printf("%c", data.in[i]);
	printf("\n");*/

	if (ioctl(cfd, CIOCCRYPT, &cryp)) {
		perror("ioctl(CIOCCRYPT)");
		exit(1);
	}

	/*printf("\nEncrypted data:\n");
	for (i = 0; i < DATA_SIZE; i++) {
	printf("%c", data.encrypted[i]);
}
printf("\n");*/

memcpy(buffer, data.encrypted, DATA_SIZE);

if (ioctl(cfd, CIOCFSESSION, &sess.ses)) {
	perror("ioctl(CIOCFSESSION)");
	exit(1);
}

if (close(cfd)<0) {
	perror("close(/dev/crypto)");
	exit(1);
}
return;
}

void decrypt(void *buffer){

	int i;
	struct session_op sess;
	struct crypt_op cryp;
	struct {
		unsigned char 	in[DATA_SIZE],
		encrypted[DATA_SIZE],
		decrypted[DATA_SIZE],
		iv[BLOCK_SIZE],
		key[KEY_SIZE];
	} data;

	memset(&sess, 0, sizeof(sess));
	memset(&cryp, 0, sizeof(cryp));

	int cfd = open("/dev/crypto", O_RDWR);
	if (cfd < 0) {
		perror("open(/dev/crypto)");
		exit(1);
	}

	//Initialize encyption key and initialization vector
	for(i=0; i<BLOCK_SIZE; i++){
		data.iv[i]='1';
		data.key[i]='0'+i;
	}
	sess.cipher = CRYPTO_AES_CBC;
	sess.keylen = KEY_SIZE;
	sess.key = data.key;

	if (ioctl(cfd, CIOCGSESSION, &sess)) {
		perror("ioctl(CIOCGSESSION)");
		exit(1);
	}

	/*
	* Encrypt data.in to data.encrypted
	*/
	cryp.ses = sess.ses;
	cryp.len = sizeof(data.in);
	memcpy(data.in, buffer, DATA_SIZE);
	cryp.src = data.in;
	cryp.dst = data.decrypted;
	cryp.iv = data.iv;
	cryp.op = COP_DECRYPT;

	/*printf("\nEncrypted data:\n");
	for (i = 0; i < DATA_SIZE; i++)
	printf("%c", data.in[i]);
	printf("\n");*/

	if (ioctl(cfd, CIOCCRYPT, &cryp)) {
		perror("ioctl(CIOCCRYPT)");
		exit(1);
	}

	/*printf("\nOriginal data:\n");
	for (i = 0; i < DATA_SIZE; i++) {
	printf("%c", data.decrypted[i]);
}
printf("\n");*/

memcpy(buffer, data.decrypted, DATA_SIZE);

if (ioctl(cfd, CIOCFSESSION, &sess.ses)) {
	perror("ioctl(CIOCFSESSION)");
	exit(1);
}

if (close(cfd)<0) {
	perror("close(/dev/crypto)");
	exit(1);
}
return;
}
