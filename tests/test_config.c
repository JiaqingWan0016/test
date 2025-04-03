/**
 * @file test_config.c
 * @brief 配置模块单元测试
 */

#include <check.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/config.h"
#include "../include/log.h"

/* 创建测试配置文件 */
static void create_test_config(const char *path, const char *content)
{
    FILE *fp = fopen(path, "w");
    if (fp) {
        fputs(content, fp);
        fclose(fp);
    }
}

/* 测试配置文件加载 */
START_TEST(test_config_load)
{
    const char *test_config_path = "/tmp/test_linkd.conf";
    
    /* 创建测试配置文件 */
    create_test_config(test_config_path, "eth0 eth1 eth2");
    
    /* 初始化日志系统 */
    log_init(NULL);
    
    /* 测试配置加载 */
    ck_assert_int_eq(config_init(), SUCCESS);
    
    /* 验证配置内容 */
    struct interface_config *cfg = config_get_interfaces();
    ck_assert_ptr_nonnull(cfg);
    ck_assert_int_eq(cfg->interface_count, 3);
    ck_assert_str_eq(cfg->interfaces[0], "eth0");
    ck_assert_str_eq(cfg->interfaces[1], "eth1");
    ck_assert_str_eq(cfg->interfaces[2], "eth2");
    
    /* 清理资源 */
    config_cleanup();
    unlink(test_config_path);
}
END_TEST

/* 创建测试套件 */
Suite *config_suite(void)
{
    Suite *s = suite_create("Config");
    TCase *tc_core = tcase_create("Core");
    
    tcase_add_test(tc_core, test_config_load);
    suite_add_tcase(s, tc_core);
    
    return s;
}

/* 主函数 */
int main(void)
{
    int number_failed;
    Suite *s = config_suite();
    SRunner *sr = srunner_create(s);
    
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
} 