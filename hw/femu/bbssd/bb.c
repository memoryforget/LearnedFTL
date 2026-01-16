#include "../nvme.h"
#include "./ld-tpftl.h"


static void bb_init_ctrl_str(FemuCtrl *n)
{
    static int fsid_vbb = 0;
    const char *vbbssd_mn = "FEMU BlackBox-SSD Controller";
    const char *vbbssd_sn = "vSSD";

    nvme_set_ctrl_name(n, vbbssd_mn, vbbssd_sn, &fsid_vbb);
}

/* bb <=> black-box */
static void bb_init(FemuCtrl *n, Error **errp)
{
    struct ssd *ssd = n->ssd = g_malloc0(sizeof(struct ssd));

    bb_init_ctrl_str(n);

    ssd->dataplane_started_ptr = &n->dataplane_started;
    ssd->ssdname = (char *)n->devname;
    femu_debug("Starting FEMU in Blackbox-SSD mode ...\n");
    ssd_init(n);
}

static void reset_stat(struct ssd *ssd)
{
    struct statistics *st = &ssd->stat;

    /*FTL*/
    // st->read_joule = 0;
    // st->write_joule = 0;
    // st->erase_joule = 0;
    // st->joule = 0;

    /*TPFTL*/
    // st->cmt_hit_cnt = 0;
    // st->cmt_miss_cnt = 0;
    // st->cmt_hit_ratio = 0;
    // st->access_cnt = 0;
    // st->read_joule = 0;
    // st->write_joule = 0;
    // st->erase_joule = 0;
    // st->joule = 0;
    
    /*LeaFTL*/
    // st->cmt_hit_cnt = 0;
    // st->cmt_miss_cnt = 0;
    // st->cmt_hit_ratio = 0;
    // st->access_cnt = 0;
    // st->model_hit = 0;
    // st->write_cnt = 0;
    // st->wa_cnt = 0;
    // st->gc_cnt = 0;
    // st->read_joule = 0;
    // st->write_joule = 0;
    // st->erase_joule = 0;
    // st->joule = 0;
    // count_segments(ssd);

    /*LearnedFTL*/
     // === 新增：重置统计开始时间 ===
     clock_gettime(CLOCK_MONOTONIC, &st->start_time);
     st->cmt_hit_cnt = 0;
     st->cmt_miss_cnt = 0;
     st->cmt_hit_ratio = 0;
     st->access_cnt = 0;
     st->model_hit_num = 0;
     st->model_use_num = 0;
     // 1. GC 相关变量置0
     st->gc_times = 0;
     st->GC_time = 0;
    
     // 2. 模型训练相关变量置0
     st->model_training_nums = 0;
     st->sort_time = 0;
     st->calculate_time = 0;
    
     // 3. 读写时延相关变量置0
     st->read_time = 0;
     st->write_time = 0;
     st->access_cnt = 0;
     st->write_num = 0;
     // 新增：清空计算平均时延所需的请求计数变量
     st->req_read_cnt = 0;   // 读请求总数
     st->req_write_cnt = 0;  // 写请求总数 
     
     st->read_joule = 0;
     st->write_joule = 0;
     st->erase_joule = 0;
     st->joule = 0;
}

static void print_stat(struct ssd *ssd)
{
    struct statistics *st = &ssd->stat;
    
    /*ftl*/
    // st->joule = st->read_joule + st->write_joule + st->erase_joule;
    // printf("read joule: %Lf\n", st->read_joule);
    // printf("write joule: %Lf\n", st->write_joule);
    // printf("erase joule: %Lf\n", st->erase_joule);
    // printf("All joule: %Lf\n", st->joule);

    /*tpftl*/
    // if (st->access_cnt == 0) {
    //     st->cmt_hit_ratio = 0;
    // } else {
    //     st->cmt_hit_ratio = (double)st->cmt_hit_cnt / st->access_cnt;
    // }
    // st->joule = st->read_joule + st->write_joule + st->erase_joule;

    // printf("CMT hit count: %lu\n", st->cmt_hit_cnt);
    // printf("CMT miss count: %lu\n", st->cmt_miss_cnt);
    // printf("CMT access count: %lu\n", st->access_cnt);
    // printf("CMT hit ratio: %lf\n", st->cmt_hit_ratio);
    // printf("read joule: %Lf\n", st->read_joule);
    // printf("write joule: %Lf\n", st->write_joule);
    // printf("erase joule: %Lf\n", st->erase_joule);
    // printf("All joule: %Lf\n", st->joule);

    /*LeaFTL*/
    // st->joule = st->read_joule + st->write_joule + st->erase_joule;
    // printf("total cnt: %lld\n", (long long)ssd->stat.access_cnt);
    // printf("cmt cnt: %lld\n", (long long)ssd->stat.cmt_hit_cnt);
    // printf("model cnt: %lld\n", (long long)ssd->stat.model_hit);
    // printf("read joule: %Lf\n", st->read_joule);
    // printf("write joule: %Lf\n", st->write_joule);
    // printf("erase joule: %Lf\n", st->erase_joule);
    // printf("All joule: %Lf\n", st->joule);
    // count_segments(ssd);

    /*LearnedFTL*/
     st->joule = st->read_joule + st->write_joule + st->erase_joule;
     printf("total cnt: %lld\n", (long long)ssd->stat.access_cnt);
     printf("cmt cnt: %lld\n", (long long)ssd->stat.cmt_hit_cnt);
     printf("model cnt: %lld\n", (long long)ssd->stat.model_hit_num);
     // 计算命中率（核心补充部分）
     long long total_cnt = (long long)ssd->stat.access_cnt;
     long long hit_cnt_sum = (long long)ssd->stat.cmt_hit_cnt + (long long)ssd->stat.model_hit_num;
     double hit_rate = 0.0;
     
     // 关键：处理除数为0的情况，避免除以0导致程序崩溃
     if (total_cnt > 0) {
         // 强制浮点运算（注意加 1.0 转为浮点数，否则整数除法会丢失精度）
         hit_rate = (double)hit_cnt_sum / total_cnt;
     } else {
         printf("Warning: total access cnt is 0, hit rate is 0\n");
     }
     
     // 打印命中率（保留4位小数，也可根据需要调整精度）
     printf("hit rate: %.4f (%.2f%%)\n", hit_rate, hit_rate * 100);
     // === 新增打印内容 ===
     // 1. GC 相关结果
     printf("GC Trigger Count: %llu\n", (long long)st->gc_times); // GC触发次数
     printf("GC Total Time (ns): %lld\n", (long long)st->GC_time); // GC总耗时
    
     // 2. 模型训练相关结果
     printf("Model Training Count: %lld\n", (long long)st->model_training_nums); // 模型训练次数
     printf("Model Sort Time (ns): %lld\n", (long long)st->sort_time); // 排序耗时
     printf("Model Calc Time (ns): %lld\n", (long long)st->calculate_time); // 计算耗时
    
     // 3. 读写时延相关结果
     printf("Total Read Latency (ns): %lld\n", (long long)st->read_time); // 总读时延
     printf("Total Write Latency (ns): %lld\n", (long long)st->write_time); // 总写时延
    
     // 如果需要平均时延，可以这样计算（需防止除零）:
     /*
     if (st->access_cnt > 0)
        printf("Avg Read Latency (ns): %lld\n", (long long)(st->read_time / st->access_cnt));
     if (st->write_num > 0)
        printf("Avg Write Latency (ns): %lld\n", (long long)(st->write_time / st->write_num)); 
     */
     // 修改平均时延计算
     if (st->req_read_cnt > 0)
     	printf("Avg Read Latency (ns): %lld\n", (long long)(st->read_time / st->req_read_cnt));
     //                 
     if (st->req_write_cnt > 0)
     	printf("Avg Write Latency (ns): %lld\n", (long long)(st->write_time / st->req_write_cnt));
     // 新增：计算并打印总平均时延（核心补充）
     // 1. 先定义临时变量，避免重复计算，代码更清晰
     long long total_time = (long long)st->read_time + (long long)st->write_time; // 总时延
     long long total_req_cnt = st->req_read_cnt + st->req_write_cnt; // 总请求数

     // 2. 计算总平均时延（必须判断总请求数是否为0，防止除零错误）
     if (total_req_cnt > 0) {
        // 注意：如果用整数除法，结果会取整；若需要更精确的小数，可改用double类型
        long long avg_total_latency = total_time / total_req_cnt;
        printf("Avg Total Latency (ns): %lld\n", avg_total_latency);

        // 可选：如果需要保留小数的高精度总平均时延（比如显示 1234.56 ns）
        double avg_total_latency_float = (double)total_time / total_req_cnt;
        printf("Avg Total Latency (ns, float): %.2f\n", avg_total_latency_float);
     } else {
        // 总请求数为0时的友好提示，避免程序崩溃
        printf("Warning: Total request count (read+write) is 0, skip avg total latency calculation\n");
     }
     // === 修改开始：IOPS 计算逻辑 ===
     struct timespec current_time;
     clock_gettime(CLOCK_MONOTONIC, &current_time);
     
     // 计算经过的时间（秒），注意纳秒部分的转换
     double elapsed_sec = (current_time.tv_sec - st->start_time.tv_sec) + (current_time.tv_nsec - st->start_time.tv_nsec) / 1000000000.0;
     // 获取总请求数（读+写）
     uint64_t total_req = st->req_read_cnt + st->req_write_cnt;
    
     double iops = 0.0;
     double read_iops = 0.0;
     double write_iops = 0.0;

     if (elapsed_sec > 0) {
        iops = (double)total_req / elapsed_sec;
        read_iops = (double)st->req_read_cnt / elapsed_sec;
        write_iops = (double)st->req_write_cnt / elapsed_sec;
     }

     printf("---------------- Performance Statistics ----------------\n");
     printf("Elapsed Time : %.4f sec\n", elapsed_sec);
     printf("Total Requests: %lu (Read: %lu, Write: %lu)\n", total_req, st->req_read_cnt, st->req_write_cnt);
     printf("Total IOPS    : %.2f\n", iops);
     printf("Read IOPS     : %.2f\n", read_iops);
     printf("Write IOPS    : %.2f\n", write_iops);
     printf("--------------------------------------------------------\n");
     printf("read joule: %Lf\n", st->read_joule);
     printf("write joule: %Lf\n", st->write_joule);
     printf("erase joule: %Lf\n", st->erase_joule);
     printf("All joule: %Lf\n", st->joule);
}

static void bb_flip(FemuCtrl *n, NvmeCmd *cmd)
{
    struct ssd *ssd = n->ssd;
    int64_t cdw10 = le64_to_cpu(cmd->cdw10);

    switch (cdw10) {
    case FEMU_ENABLE_GC_DELAY:
        ssd->sp.enable_gc_delay = true;
        femu_log("%s,FEMU GC Delay Emulation [Enabled]!\n", n->devname);
        break;
    case FEMU_DISABLE_GC_DELAY:
        ssd->sp.enable_gc_delay = false;
        femu_log("%s,FEMU GC Delay Emulation [Disabled]!\n", n->devname);
        break;
    case FEMU_ENABLE_DELAY_EMU:
        ssd->sp.pg_rd_lat = NAND_READ_LATENCY;
        ssd->sp.pg_wr_lat = NAND_PROG_LATENCY;
        ssd->sp.blk_er_lat = NAND_ERASE_LATENCY;
        ssd->sp.ch_xfer_lat = 0;
        femu_log("%s,FEMU Delay Emulation [Enabled]!\n", n->devname);
        break;
    case FEMU_DISABLE_DELAY_EMU:
        ssd->sp.pg_rd_lat = 0;
        ssd->sp.pg_wr_lat = 0;
        ssd->sp.blk_er_lat = 0;
        ssd->sp.ch_xfer_lat = 0;
        femu_log("%s,FEMU Delay Emulation [Disabled]!\n", n->devname);
        break;
    case FEMU_RESET_ACCT:
        n->nr_tt_ios = 0;
        n->nr_tt_late_ios = 0;
        femu_log("%s,Reset tt_late_ios/tt_ios,%lu/%lu\n", n->devname,
                n->nr_tt_late_ios, n->nr_tt_ios);
        break;
    case FEMU_ENABLE_LOG:
        n->print_log = true;
        femu_log("%s,Log print [Enabled]!\n", n->devname);
        break;
    case FEMU_DISABLE_LOG:
        n->print_log = false;
        femu_log("%s,Log print [Disabled]!\n", n->devname);
        break;
    case FEMU_RESET_STAT:
        reset_stat(ssd);
        femu_log("%s,Reset statistics!\n", n->devname);
        break;
    case FEMU_PRINT_STAT:
        print_stat(ssd);
        femu_log("%s,Statistics print!\n", n->devname);
        break;
    default:
        printf("FEMU:%s,Not implemented flip cmd (%lu)\n", n->devname, cdw10);
    }
}

static uint16_t bb_nvme_rw(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd,
                           NvmeRequest *req)
{
    return nvme_rw(n, ns, cmd, req);
}

static uint16_t bb_io_cmd(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd,
                          NvmeRequest *req)
{
    switch (cmd->opcode) {
    case NVME_CMD_READ:
    case NVME_CMD_WRITE:
        return bb_nvme_rw(n, ns, cmd, req);
    default:
        return NVME_INVALID_OPCODE | NVME_DNR;
    }
}

static uint16_t bb_admin_cmd(FemuCtrl *n, NvmeCmd *cmd)
{
    switch (cmd->opcode) {
    case NVME_ADM_CMD_FEMU_FLIP:
        bb_flip(n, cmd);
        return NVME_SUCCESS;
    default:
        return NVME_INVALID_OPCODE | NVME_DNR;
    }
}

int nvme_register_bbssd(FemuCtrl *n)
{
    n->ext_ops = (FemuExtCtrlOps) {
        .state            = NULL,
        .init             = bb_init,
        .exit             = NULL,
        .rw_check_req     = NULL,
        .admin_cmd        = bb_admin_cmd,
        .io_cmd           = bb_io_cmd,
        .get_log          = NULL,
    };

    return 0;
}

