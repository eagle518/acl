#pragma once
#include <list>
#include "acl_cpp/stdlib/locker.hpp"

namespace acl
{

class connect_client;

/**
 * �ͻ������ӳ��࣬ʵ�ֶ����ӳصĶ�̬����������Ϊ�����࣬��Ҫ����ʵ��
 * ���麯�� create_connect ���ڴ��������˵�һ������
 */
class connect_pool
{
public:
	/**
	 * ���캯��
	 * @param addr {const char*} ������������ַ����ʽ��ip:port(domain:port)
	 * @param count {int} ���ӳ�������Ӹ�������
	 * @param retry_inter {int} �����ӶϿ��������ٴδ����ӵ�ʱ����(��)��
	 *  ����ֵ <= 0 ʱ��ʾ�������ӶϿ����������������������볬����ʱ�������
	 *  �����Ͽ�����
	 */
	connect_pool(const char* addr, int count, int retry_inter = 0);
	virtual ~connect_pool();

	/**
	 * �������ӳ��п������ӵĿ�����������
	 * @param ttl {time_t} �������ӵ��������ڣ�����ֵ < 0 ���ʾ�������Ӳ����ڣ�
	 *  == 0 ʱ��ʾ���̹��ڣ�> 0 ��ʾ���и�ʱ��κ󽫱��ͷ�
	 * @return {connect_pool&}
	 */
	connect_pool& set_idle_ttl(time_t ttl);

	/**
	 * �����ӳ��г����Ի�ȡһ�����ӣ��������������á����ϴη���������쳣ʱ����
	 * δ���ڻ����ӳ����Ӹ����ﵽ���������򽫷��� NULL��������һ���µ����������
	 * ����ʱʧ�ܣ�������ӳػᱻ��Ϊ������״̬
	 * @return {connect_client*} ���Ϊ�����ʾ�÷��������ӳض��󲻿���
	 */
	connect_client* peek();

	/**
	 * �ͷ�һ�����������ӳ��У��������ӳض�Ӧ�ķ����������û������ϣ���رո�����ʱ��
	 * ������ӽ��ᱻֱ���ͷ�
	 * @param conn {redis_client*}
	 * @param keep {bool} �Ƿ���Ը����ӱ��ֳ�����
	 */
	void put(connect_client* conn, bool keep = true);

	/**
	 * ������ӳ��п��е����ӣ������ڵ������ͷŵ�
	 * @param ttl {time_t} ����ʱ����������ֵ�����ӽ����ͷ�
	 * @param exclusive {bool} �ڲ��Ƿ���Ҫ����
	 * @return {int} ���ͷŵĿ������Ӹ���
	 */
	int check_idle(time_t ttl, bool exclusive = true);

	/**
	 * �������ӳصĴ��״̬
	 * @param ok {bool} ���ø������Ƿ�����
	 */
	void set_alive(bool ok /* true | false */);

	/**
	 * ��ȡ���ӳصķ�������ַ
	 * @return {const char*} ���طǿյ�ַ
	 */
	const char* get_addr() const
	{
		return addr_;
	}

	/**
	 * ��ȡ���ӳ��������������
	 * @return {int}
	 */
	int get_count() const
	{
		return count_;
	}

	/**
	 * ����ͳ�Ƽ�����
	 * @param inter {int} ͳ�Ƶ�ʱ����
	 */
	void reset_statistics(int inter)
	{
		time_t now = time(NULL);
		lock_.lock();
		if (now - last_ >= inter)
		{
			last_ = now;
			current_used_ = 0;
		}
		lock_.unlock();
	}

	/**
	 * ��ȡ�����ӳ��ܹ���ʹ�õĴ���
	 */
	unsigned long long get_total_used() const
	{
		return total_used_;
	}

	/**
	 * ��ȡ�����ӳص�ǰ��ʹ�ô���
	 * @return {unsigned long long}
	 */
	unsigned long long get_current_used() const
	{
		return current_used_;
	}

protected:
	virtual connect_client* create_connect() = 0;

protected:
	// �Ƿ�������
	bool  alive_;
	// ������ķ������Ŀ������Ե�ʱ���������������ӳض����ٴα����õ�ʱ����
	int   retry_inter_;
	time_t last_dead_;			// �����ӳض����ϴβ�����ʱ��ʱ���
	char  addr_[64];			// ���ӳض�Ӧ�ķ�������ַ��IP:PORT
	int   max_;				// ���������
	int   count_;				// ��ǰ��������
	time_t idle_ttl_;			// �������ӵ���������
	time_t last_check_;			// �ϴμ��������ӵ�ʱ���
	int   check_inter_;			// ���������ӵ�ʱ����

	locker lock_;				// ���� pool_ ʱ�Ļ�����
	unsigned long long total_used_;		// �����ӳص����з�����
	unsigned long long current_used_;	// ĳʱ����ڵķ�����
	time_t last_;				// �ϴμ�¼��ʱ���
	std::list<connect_client*> pool_;	// ���ӳؼ���
};

} // namespace acl