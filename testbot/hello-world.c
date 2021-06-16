/* Attestation API test. Only works for SGX PAL. */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sgx_api.h"
#include "sgx_arch.h"
#include "sgx_attest.h"

uint8_t g_quote[SGX_QUOTE_MAX_SIZE];

char user_report_data_str[] = "This is user-provided report data";

enum
{
    SUCCESS = 0,
    FAILURE = -1
};

ssize_t (*rw_file_f)(const char *path, char *buf, size_t bytes, bool do_write);

static ssize_t rw_file_posix(const char *path, char *buf, size_t bytes, bool do_write)
{
    ssize_t rv = 0;
    ssize_t ret = 0;

    int fd = open(path, do_write ? O_WRONLY : O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "opening %s failed\n", path);
        return fd;
    }

    while (bytes > rv)
    {
        if (do_write)
            ret = write(fd, buf + rv, bytes - rv);
        else
            ret = read(fd, buf + rv, bytes - rv);

        if (ret > 0)
        {
            rv += ret;
        }
        else if (ret == 0)
        {
            /* end of file */
            if (rv == 0)
                fprintf(stderr, "%s failed: unexpected end of file\n", do_write ? "write" : "read");
            break;
        }
        else
        {
            if (ret < 0 && (errno == EAGAIN || errno == EINTR))
            {
                continue;
            }
            else
            {
                fprintf(stderr, "%s failed: %s\n", do_write ? "write" : "read", strerror(errno));
                goto out;
            }
        }
    }

out:
    if (ret < 0)
    {
        /* error path */
        close(fd);
        return ret;
    }

    ret = close(fd);
    if (ret < 0)
    {
        fprintf(stderr, "closing %s failed\n", path);
        return ret;
    }
    return rv;
}

static ssize_t rw_file_stdio(const char *path, char *buf, size_t bytes, bool do_write)
{
    size_t rv = 0;
    size_t ret = 0;

    FILE *f = fopen(path, do_write ? "wb" : "rb");
    if (!f)
    {
        fprintf(stderr, "opening %s failed\n", path);
        return -1;
    }

    while (bytes > rv)
    {
        if (do_write)
            ret = fwrite(buf + rv, /*size=*/1, /*nmemb=*/bytes - rv, f);
        else
            ret = fread(buf + rv, /*size=*/1, /*nmemb=*/bytes - rv, f);

        if (ret > 0)
        {
            rv += ret;
        }
        else
        {
            if (feof(f))
            {
                if (rv)
                {
                    /* read some bytes from file, success */
                    break;
                }
                assert(rv == 0);
                fprintf(stderr, "%s failed: unexpected end of file\n", do_write ? "write" : "read");
                fclose(f);
                return -1;
            }

            assert(ferror(f));

            if (errno == EAGAIN || errno == EINTR)
            {
                continue;
            }

            fprintf(stderr, "%s failed: %s\n", do_write ? "write" : "read", strerror(errno));
            fclose(f);
            return -1;
        }
    }

    int close_ret = fclose(f);
    if (close_ret)
    {
        fprintf(stderr, "closing %s failed\n", path);
        return -1;
    }
    return rv;
}

/*!
 * \brief Test quote interface (currently SGX quote obtained from the Quoting Enclave).
 *
 * Perform the following steps in order:
 *   1. write some custom data to `user_report_data` file
 *   2. read `quote` file
 *   3. verify report data read from `quote`
 *
 * \return 0 if the test succeeds, -1 otherwise.
 */
static int test_quote_interface(void)
{
    ssize_t bytes;

    /* 1. write some custom data to `user_report_data` file */
    sgx_report_data_t user_report_data = {0};
    static_assert(sizeof(user_report_data) >= sizeof(user_report_data_str),
                  "insufficient size of user_report_data");

    memcpy((void *)&user_report_data, (void *)user_report_data_str, sizeof(user_report_data_str));

    bytes = rw_file_f("/dev/attestation/user_report_data", (char *)&user_report_data,
                      sizeof(user_report_data), /*do_write=*/true);
    if (bytes != sizeof(user_report_data))
    {
        /* error is already printed by rw_file_f() */
        return FAILURE;
    }

    /* 2. read `quote` file */
    bytes = rw_file_f("/dev/attestation/quote", (char *)&g_quote, sizeof(g_quote),
                      /*do_write=*/false);
    if (bytes < 0)
    {
        /* error is already printed by rw_file_f() */
        return FAILURE;
    }

    /* 3. verify report data read from `quote` */
    if (bytes < sizeof(sgx_quote_t))
    {
        fprintf(stderr, "obtained SGX quote is too small: %ldB (must be at least %ldB)\n", bytes,
                sizeof(sgx_quote_t));
        return FAILURE;
    }

    sgx_quote_t *typed_quote = (sgx_quote_t *)g_quote;

    if (typed_quote->version != /*EPID*/ 2 && typed_quote->version != /*DCAP*/ 3)
    {
        fprintf(stderr, "version of SGX quote is not EPID (2) and not ECDSA/DCAP (3)\n");
        return FAILURE;
    }

    int ret = memcmp(typed_quote->report_body.report_data.d, user_report_data.d,
                     sizeof(user_report_data));
    if (ret)
    {
        fprintf(stderr, "comparison of report data in SGX quote failed\n");
        return FAILURE;
    }

    //printf("quote.len = %ld\n", bytes);

    return SUCCESS;
}

int main(int argc, char **argv)
{
    rw_file_f = rw_file_posix;
    if (argc > 1)
    {
        /* simple trick to test stdio-style interface to pseudo-files in our tests */
        rw_file_f = rw_file_stdio;
    }

    printf("Test quote interface... %s\n",
           test_quote_interface() == SUCCESS ? "SUCCESS" : "FAIL");
    return 0;
}
