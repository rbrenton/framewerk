CREATE TABLE `cron` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `hostname` varchar(255) DEFAULT NULL,
  `ami` varchar(16) DEFAULT NULL,
  `user` varchar(255) NOT NULL,
  `m` varchar(255) NOT NULL DEFAULT '*',
  `h` varchar(255) NOT NULL DEFAULT '*',
  `dom` varchar(255) NOT NULL DEFAULT '*',
  `mon` varchar(255) NOT NULL DEFAULT '*',
  `dow` varchar(255) NOT NULL DEFAULT '*',
  `jenkins_job` varchar(255) DEFAULT NULL,
  `workdir` varchar(255) NOT NULL,
  `command` varchar(255) NOT NULL,
  `do_concurrent` tinyint(1) NOT NULL DEFAULT '0',
  `do_sequential` tinyint(1) NOT NULL DEFAULT '1',
  `suspended` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1000 DEFAULT CHARSET=utf8;

CREATE TABLE `cron_status` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `cron_id` int(10) unsigned NOT NULL,
  `hostname` varchar(255) NOT NULL,
  `time_start` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `pid` int(10) unsigned DEFAULT NULL,
  `time_end` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `key_cron_host` (`cron_id`,`hostname`),
  KEY `idx_fk_cron_status` (`cron_id`),
  CONSTRAINT `fk_cron_status` FOREIGN KEY (`cron_id`) REFERENCES `cron` (`id`) ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=10000 DEFAULT CHARSET=utf8;
