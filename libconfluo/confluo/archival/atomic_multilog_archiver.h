#ifndef CONFLUO_ATOMIC_MULTILOG_ARCHIVAL_ARCHIVER_H_
#define CONFLUO_ATOMIC_MULTILOG_ARCHIVAL_ARCHIVER_H_

#include "container/data_log.h"
#include "monolog_linear_archiver.h"
#include "filter_log.h"
#include "filter_log_archiver.h"
#include "index_log.h"
#include "index_log_archiver.h"
#include "read_tail.h"

namespace confluo {
namespace archival {

class atomic_multilog_archiver {
 public:
  atomic_multilog_archiver()
      : atomic_multilog_archiver("", read_tail(), nullptr, nullptr, nullptr, nullptr, false) {
  }

  /**
   * Constructor.
   * @param path directory to store archives in or where archives are currently stored
   * @param rt data log read tail
   * @param log data log
   * @param filters atomic multilog filters
   * @param indexes atomic multilog indexes
   * @param schema data log schema
   * @param clear
   */
  atomic_multilog_archiver(const std::string& path, read_tail rt, data_log* log,
           filter_log* filters, index_log* indexes, schema_t* schema,
           bool clear = true)
      : path_(path),
        rt_(rt),
        record_size_(schema->record_size()) {
    if (clear) {
      file_utils::clear_dir(data_log_path());
      file_utils::clear_dir(filter_log_path());
      file_utils::clear_dir(index_log_path());
    }
    data_log_archiver_ = data_log_archiver(data_log_path(), log);
    filter_log_archiver_ = filter_log_archiver(filter_log_path(), filters);
    index_log_archiver_ = index_log_archiver(index_log_path(), indexes, schema);
  }

  /**
   * Attempt to archive data log, filters and indexes
   * up to a particular data log offset. Best effort.
   * Does not allow for archival past the read tail.
   * @param offset data log offset
   */
  void archive(size_t offset) {
    offset = std::min(offset - offset % record_size_, (size_t) rt_.get());
    std::ofstream test("archival_out");
    if (offset > data_log_archiver_.tail()) {
      uint64_t a = time_utils::cur_ns();
      LOG_INFO << "i am here.alya0";
      data_log_archiver_.archive(offset);
      LOG_INFO << "i am here.alya";
      uint64_t b = time_utils::cur_ns();
      filter_log_archiver_.archive(offset);
      uint64_t c = time_utils::cur_ns();
      index_log_archiver_.archive(offset);
      uint64_t d = time_utils::cur_ns();
      LOG_INFO << (b - a);
      std::cerr << "Data log archival: " << (b - a) << "\n";
      std::cout << "Data log archival: " << (b - a) << "\n";
      std::cerr << "Filter log archival: " << (c - b) << "\n";
      std::cout << "Filter log archival: " << (c - b) << "\n";
      std::cerr << "Index log archival: " << (d - c) << "\n";
      std::cout << "Index log archival: " << (d - c) << "\n";
      std::cerr.flush();
      std::cout.flush();
      test << (b - a) << "\n";
      test << (c - b) << "\n";
      test << (d - c) << "\n";
    }
  }

  /**
   *
   * @return offset up to which data log has been archived
   */
  size_t tail() {
    return data_log_archiver_.tail();
  }

  /**
   *
   * @return path of directory in which data log archives are stored
   */
  std::string data_log_path() {
    return path_ + "/archives/data_log/";
  }

  /**
   *
   * @return path of directory in which filter log archives are stored
   */
  std::string filter_log_path() {
    return path_ + "/archives/filters/";
  }

  /**
   *
   * @return path of directory in which index log archives are stored
   */
  std::string index_log_path() {
    return path_ + "/archives/indexes/";
  }

 private:
  std::string path_;
  read_tail rt_;
  size_t record_size_;
  data_log_archiver data_log_archiver_;
  filter_log_archiver filter_log_archiver_;
  index_log_archiver index_log_archiver_;

};

}
}

#endif /* CONFLUO_ATOMIC_MULTILOG_ARCHIVAL_ARCHIVER_H_ */
