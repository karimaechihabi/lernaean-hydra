typedef struct file_access_manager {
    FILE *file;
    char *path;
    unsigned int page_size;
    unsigned int record_size;
    unsigned int records_per_page;

    char *buffer;

    unsigned long number_of_pages;
    unsigned long last_page_utilization;
} file_access_manager;

/**
 * Open file etc...
 */
int initialize_file_access_manager(char *filename, unsigned int page_size, unsigned int record_size, unsigned long buffer_size_pages);
    
/**
 * Process: load page (if not loaded) and get the specified record.
 */
void read_record(file_access_manager *manager, unsigned long page_id, unsigned long record_id, char *destination);

/**
 * Process: load page (if not loaded) update the specified record, mark as to flush
 */
void write_record(file_access_manager *manager, unsigned long page_id, unsigned long record_id, char *source);

/**
 * Process: find last page, if there is space append something, else create new page and append this
 */
void append_record(file_access_manager *manager, char *source);

/**
 * Read page in buffer
 */
void read_page(file_access_manager *manager, unsigned long page_id, char *destination);

/**
 * Write page from buffer to disk
 */
void write_page(file_access_manager *manager, unsigned long page_id, char *source);

/**
 * Flush all changed pages to disk
 */
void flush_all_pages(file_access_manager *manager);
