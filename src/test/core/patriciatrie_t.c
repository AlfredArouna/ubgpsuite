/* Copyright (C) 2019 Alpha Cogs S.R.L.
 *
 * The ubgp library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The ubgp library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the ubgp library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This work is based upon work authored by the Institute of Informatics
 * and Telematics of the Italian National Research Council (IIT-CNR) licensed
 * under the BSD 3-Clause license. See AKNOWLEDGEMENT and AUTHORS for more
 * details.
 */

#include "../../ubgp/patriciatrie.h"
#include "../../ubgp/ubgpdef.h"
#include "../test_util.h"
#include "test.h"

#include <CUnit/CUnit.h>

#include <stdlib.h>

static netaddr_t *pfx(const char *s)
{
    static netaddr_t p;

    CU_ASSERT_FATAL(stonaddr(&p, s) == 0);
    return &p;
}

void testpatbase(void)
{
    patricia_trie_t pt;
    patinit(&pt, AF_INET);

    int inserted;

    trienode_t *n = patinsert(&pt, pfx("8.2.0.0/16"), &inserted);
    CU_ASSERT_FATAL(n != NULL);
    CU_ASSERT(inserted == PREFIX_INSERTED);
    CU_ASSERT(n->prefix.family == AF_INET);
    CU_ASSERT(strcmp(naddrtos(&n->prefix, NADDR_CIDR), "8.2.0.0/16") == 0);

    trienode_t *m = patsearchexact(&pt, pfx("8.2.0.0/16"));
    CU_ASSERT(m == n);

    n = patinsert(&pt, pfx("9.2.0.0/16"), &inserted);
    CU_ASSERT_FATAL(n != NULL);
    CU_ASSERT(inserted == PREFIX_INSERTED);
    CU_ASSERT(n->prefix.family == AF_INET);
    CU_ASSERT(strcmp(naddrtos(&n->prefix, NADDR_CIDR), "9.2.0.0/16") == 0);

    m = patsearchexact(&pt, pfx("9.2.0.0/16"));
    CU_ASSERT(m == n);

    n = patsearchbest(&pt, pfx("8.2.2.0/24"));

    CU_ASSERT_FATAL(n != NULL);
    CU_ASSERT(inserted == PREFIX_INSERTED);
    CU_ASSERT(n->prefix.family == AF_INET);
    CU_ASSERT(strcmp(naddrtos(&n->prefix, NADDR_CIDR), "8.2.0.0/16") == 0);

    patremove(&pt, pfx("8.2.0.0/16"));
    m = patsearchexact(&pt, pfx("8.2.0.0/16"));
    CU_ASSERT(m == NULL);

    patdestroy(&pt);
}

void testpatgetfuncs(void)
{
    patricia_trie_t pt;
    patinit(&pt, AF_INET);

    patinsert(&pt, pfx("8.0.0.0/8"), NULL);
    patinsert(&pt, pfx("8.2.0.0/16"), NULL);
    patinsert(&pt, pfx("8.2.2.0/24"), NULL);
    patinsert(&pt, pfx("8.2.2.1/32"), NULL);
    patinsert(&pt, pfx("9.2.2.1/32"), NULL);

    printf("Inserted:\n%s\n%s\n%s\n%s\n%s\n", "8.0.0.0/8", "8.2.0.0/16", "8.2.2.0/24", "8.2.2.1/32", "9.2.2.1/32");

    trienode_t** supernets = patgetsupernetsof(&pt, pfx("8.2.2.1/32"));
    CU_ASSERT_FATAL(supernets != NULL);
    CU_ASSERT_FATAL(supernets[0] != NULL);
    CU_ASSERT_FATAL(supernets[1] != NULL);
    CU_ASSERT_FATAL(supernets[2] != NULL);
    CU_ASSERT_FATAL(supernets[3] != NULL);

    printf("Supernets of 8.2.2.1/32:\n");
    uint i;
    for (i = 0; supernets[i] != NULL; i++) {
        printf("%s\n", naddrtos(&supernets[i]->prefix, NADDR_CIDR));
    }
    printf("--\n");

    CU_ASSERT(i == 4);
    CU_ASSERT(supernets[4] == NULL);
    free(supernets);

    trienode_t** subnets = patgetsubnetsof(&pt, pfx("8.0.0.0/8"));
    printf("Subnets of 8.0.0.0/8:\n");
    for (i = 0; subnets[i] != NULL; i++) {
        printf("%s\n", naddrtos(&subnets[i]->prefix, NADDR_CIDR));
    }
    printf("--\n");
    free(subnets);

    trienode_t** related = patgetrelatedof(&pt, pfx("8.2.2.0/24"));
    printf("Related of 8.2.2.0/24:\n");
    for (i = 0; related[i] != NULL; i++) {
        printf("%s\n", naddrtos(&related[i]->prefix, NADDR_CIDR));
    }
    printf("--\n");
    free(related);

    supernets = patgetsupernetsof(&pt, pfx("9.2.2.1/32"));
    CU_ASSERT_FATAL(supernets != NULL);
    CU_ASSERT_FATAL(supernets[0] != NULL);
    CU_ASSERT_FATAL(supernets[1] == NULL);
    free(supernets);

    patdestroy(&pt);
}

void testpatcheckfuncs(void)
{
    patricia_trie_t pt;
    patinit(&pt, AF_INET);

    patinsert(&pt, pfx("8.0.0.0/8"), NULL);

    CU_ASSERT(patissubnetof(&pt, pfx("8.2.2.1/32")));
    CU_ASSERT(!patissupernetof(&pt, pfx("8.2.2.1/32")));

    patinsert(&pt, pfx("9.2.0.0/16"), NULL);
    CU_ASSERT(patissupernetof(&pt, pfx("9.0.0.0/8")) == 1);
    CU_ASSERT(patissubnetof(&pt, pfx("9.2.2.0/24")) == 1);

    patdestroy(&pt);

    patricia_trie_t p;
    patinit(&p, AF_INET);
    patinsert(&p, pfx("132.160.0.0/17"), NULL);
    patclear(&p);

    patinsert(&p, pfx("132.160.0.0/17"), NULL);
    patinsert(&p, pfx("168.105.0.0/16"), NULL);
    patsearchexact(&p, pfx("205.72.240.0/20"));
    patdestroy(&p);
}

void testpatcoverage(void)
{
    patricia_trie_t pt;
    patinit(&pt, AF_INET);

    patinsert(&pt, pfx("0.0.0.0/0"), NULL);
    patinsert(&pt, pfx("8.0.0.0/8"), NULL);

    u128 coverage = patcoverage(&pt);
    CU_ASSERT(u128cmpu(coverage, 16777216) == 0);

    patinsert(&pt, pfx("8.2.0.0/16"), NULL);
    coverage = patcoverage(&pt);
    CU_ASSERT(u128cmpu(coverage, 16777216) == 0);

    patinsert(&pt, pfx("9.0.0.0/8"), NULL);
    coverage = patcoverage(&pt);
    CU_ASSERT(u128cmpu(coverage, 33554432) == 0);

    patdestroy(&pt);

    patinit(&pt, AF_INET6);

    patinsert(&pt, pfx("0.0.0.0/0"), NULL);
    patinsert(&pt, pfx("2a00::/8"), NULL);

    coverage = patcoverage(&pt);
    u128 expected = u128shl(U128_ONE, 120);
    CU_ASSERT(u128cmp(coverage, expected) == 0);

    patdestroy(&pt);
}

void testpatgetfirstsubnets(void)
{
    patricia_trie_t pt;
    patinit(&pt, AF_INET);

    patinsert(&pt, pfx("0.0.0.0/0"), NULL);
    patinsert(&pt, pfx("8.0.0.0/8"), NULL);

    trienode_t **firstsubnets = patgetfirstsubnetsof(&pt, pfx("0.0.0.0/0"));

    CU_ASSERT_FATAL(firstsubnets != NULL);
    CU_ASSERT(firstsubnets[0] != NULL);
    CU_ASSERT(firstsubnets[1] == NULL);

    CU_ASSERT(strcmp("8.0.0.0/8", naddrtos(&firstsubnets[0]->prefix, NADDR_CIDR)) == 0);

    free(firstsubnets);

    patdestroy(&pt);
}

void testpatiterator(void)
{
    patricia_trie_t pt;
    patinit(&pt, AF_INET);

    patinsert(&pt, pfx("0.0.0.0/0"), NULL);
    patinsert(&pt, pfx("8.0.0.0/8"), NULL);
    patinsert(&pt, pfx("8.2.0.0/16"), NULL);
    patinsert(&pt, pfx("8.2.2.0/24"), NULL);
    patinsert(&pt, pfx("8.2.2.1/32"), NULL);
    patinsert(&pt, pfx("9.2.2.1/32"), NULL);
    patinsert(&pt, pfx("128.2.2.1/32"), NULL);

    printf("\n");

    patiterator_t it;

    patiteratorinit(&it, &pt);
    while (!patiteratorend(&it)) {
        trienode_t *node = patiteratorget(&it);
        printf("%s\n", naddrtos(&node->prefix, NADDR_CIDR));
        patiteratornext(&it);
    }

    patdestroy(&pt);
}

void testpatproblem(void)
{
    static const char *const prefixes[] = {
        "199.245.187.0/24",
        "120.50.4.0/23",
        "207.179.89.0/24",
        "103.50.254.0/24",
        "103.198.184.0/24",
        "195.225.34.0/23",
        "103.250.60.0/23",
        "203.55.144.0/24",
        "123.50.80.0/23",
        "207.179.89.0/24",
        "123.50.64.0/18",
        "103.50.254.0/24",
        "43.241.99.0/24",
        "195.225.34.0/23",
        "203.55.144.0/24",
        "203.57.91.0/24",
        "103.50.254.0/24",
        "120.50.4.0/23",
        "103.50.254.0/24",
        "199.245.187.0/24",
        "103.198.184.0/24",
        "81.85.191.0/24",
        "103.50.254.0/24",
        "199.245.187.0/24",
        "46.149.48.0/23",
        "46.149.52.0/23",
        "46.149.60.0/22",
        "81.85.191.0/24",
        "103.50.254.0/24",
        "170.0.5.0/24",
        "170.0.6.0/24",
        "170.0.7.0/24",
        "103.50.254.0/24",
        "103.250.60.0/23",
        "103.198.184.0/24",
        "103.198.185.0/24",
        "202.3.242.0/23",
        "103.250.60.0/23",
        "103.50.254.0/24",
        "103.50.254.0/24",
        "199.245.187.0/24",
        "27.122.16.0/20",
        "103.3.168.0/22",
        "202.95.192.0/20",
        "46.149.48.0/23",
        "46.149.52.0/23",
        "46.149.60.0/22",
        "103.50.254.0/24",
        "103.43.146.0/24",
        "202.1.48.0/20",
        "202.58.128.0/22",
        "202.58.131.0/24",
        "202.61.0.0/24",
        "202.165.203.0/24",
        "124.240.214.0/23",
        "103.49.207.0/24",
        "103.77.24.0/23",
        "124.240.212.0/23",
        "103.3.168.0/24",
        "103.3.169.0/24",
        "202.95.206.0/24",
        "103.15.114.0/24",
        "103.15.115.0/24",
        "124.240.201.0/24",
        "124.240.202.0/24",
        "103.242.164.0/24",
        "124.240.192.0/19",
        "27.122.16.0/24",
        "27.122.20.0/24",
        "27.122.21.0/24",
        "27.122.23.0/24",
        "27.122.24.0/24",
        "27.122.25.0/24",
        "27.122.26.0/24",
        "27.122.27.0/24",
        "27.122.28.0/24",
        "27.122.29.0/24",
        "27.122.31.0/24",
        "202.95.195.0/24",
        "202.95.197.0/24",
        "202.95.198.0/24",
        "202.95.199.0/24",
        "202.95.200.0/24",
        "202.95.201.0/24",
        "202.95.203.0/24",
        "202.95.204.0/24",
        "202.95.207.0/24",
        "27.122.30.0/24",
        "202.95.192.0/24",
        "202.95.193.0/24",
        "202.95.194.0/24",
        "14.192.72.0/22",
        "103.20.76.0/22",
        "103.110.31.0/24",
        "124.240.200.0/23",
        "103.198.184.0/24",
        "103.198.185.0/24",
        "120.50.4.0/23",
        "103.50.254.0/24",
        "103.250.60.0/23",
        "207.179.73.0/24",
        "69.89.110.0/24",
        "69.89.123.0/24",
        "207.179.89.0/24",
        "199.245.187.0/24",
        "103.50.254.0/24",
        "203.57.91.0/24",
        "69.89.110.0/24",
        "69.89.123.0/24",
        "207.179.73.0/24",
        "207.179.89.0/24",
        "199.245.187.0/24",
        "103.50.254.0/24",
        "46.149.48.0/23",
        "46.149.52.0/23",
        "46.149.60.0/22",
        "103.50.254.0/24",
        "120.50.4.0/23",
        "103.198.185.0/24",
        "103.99.174.0/23",
        "185.59.252.0/22",
        "103.50.254.0/24",
        "103.50.254.0/24",
        "199.245.187.0/24",
        "185.59.252.0/22",
        "199.245.187.0/24",
        "103.50.254.0/24",
        "46.149.48.0/23",
        "46.149.52.0/23",
        "46.149.60.0/22",
        "103.198.184.0/24",
        "203.57.91.0/24",
        "120.50.4.0/23",
        "103.50.254.0/24",
        "203.57.91.0/24",
        "31.148.20.0/24",
        "103.198.185.0/24",
        "185.59.252.0/22",
        "43.241.99.0/24",
        "103.50.254.0/24",
        "199.245.187.0/24",
        "196.201.218.0/24",
        "196.201.221.0/24",
        "196.201.208.0/24",
        "196.201.208.0/20",
        "196.96.0.0/13",
        "196.104.0.0/13",
        "154.118.233.0/24",
        "154.231.0.0/17",
        "194.9.64.0/24",
        "196.3.57.0/24",
        "196.6.226.0/24",
        "196.8.225.0/24",
        "196.20.128.0/17",
        "196.20.132.0/24",
        "196.20.196.0/24",
        "196.20.212.0/24",
        "196.27.64.0/19",
        "196.192.0.0/20",
        "196.192.5.0/24",
        "196.192.10.0/24",
        "196.192.96.0/20",
        "197.224.0.0/14",
        "197.224.6.0/24",
        "197.224.7.0/24",
        "197.224.128.0/17",
        "197.224.228.0/24",
        "197.224.229.0/24",
        "197.224.230.0/24",
        "197.225.0.0/19",
        "197.225.13.0/24",
        "197.225.14.0/24",
        "197.225.15.0/24",
        "197.225.128.0/18",
        "197.225.182.0/24",
        "197.225.183.0/24",
        "197.226.0.0/18",
        "197.226.39.0/24",
        "197.226.64.0/18",
        "197.227.0.0/16",
        "197.227.18.0/24",
        "197.227.159.0/24",
        "202.60.0.0/21",
        "202.123.0.0/19",
        "202.123.26.0/24",
        "196.10.119.0/24",
        "196.13.173.0/24",
        "196.43.205.0/24",
        "196.43.241.0/24",
        "196.45.120.0/23",
        "196.50.21.0/24",
        "196.96.0.0/12",
        "196.201.212.0/22",
        "196.201.216.0/23",
        "197.176.0.0/13",
        "197.239.36.0/24",
        "197.248.0.0/22",
        "197.248.0.0/18",
        "197.248.3.0/24",
        "197.248.10.0/24",
        "197.248.16.0/24",
        "197.248.17.0/24",
        "197.248.20.0/24",
        "197.248.23.0/24",
        "197.248.24.0/24",
        "197.248.25.0/24",
        "197.248.27.0/24",
        "197.248.28.0/24",
        "197.248.29.0/24",
        "197.248.31.0/24",
        "197.248.36.0/24",
        "197.248.40.0/24",
        "197.248.44.0/24",
        "197.248.59.0/24",
        "197.248.61.0/24",
        "197.248.64.0/24",
        "197.248.64.0/18",
        "197.248.70.0/24",
        "197.248.80.0/24",
        "197.248.84.0/24",
        "197.248.87.0/24",
        "197.248.100.0/24",
        "197.248.118.0/24",
        "197.248.123.0/24",
        "197.248.125.0/24",
        "197.248.127.0/24",
        "197.248.128.0/18",
        "197.248.133.0/24",
        "197.248.134.0/24",
        "197.248.135.0/24",
        "197.248.143.0/24",
        "197.248.144.0/24",
        "197.248.148.0/24",
        "197.248.152.0/24",
        "197.248.154.0/24",
        "197.248.161.0/24",
        "197.248.163.0/24",
        "197.248.170.0/24",
        "197.248.183.0/24",
        "197.248.184.0/24",
        "197.248.192.0/18",
        "41.90.80.0/21",
        "41.90.88.0/22",
        "41.90.128.0/20",
        "41.90.128.0/18",
        "41.90.144.0/20",
        "41.90.160.0/20",
        "41.90.176.0/20",
        "41.90.192.0/18",
        "41.139.128.0/17",
        "41.203.208.0/20",
        "197.248.0.0/16",
        "213.150.115.0/24",
        "41.79.80.0/22",
        "154.117.128.0/18",
        "154.117.168.0/24",
        "154.117.175.0/24",
        "154.127.112.0/20",
        "154.127.118.0/24",
        "197.234.192.0/24",
        "197.234.192.0/21",
        "154.117.176.0/24",
        "197.234.193.0/24",
        "41.223.152.0/22",
        "160.226.192.0/18",
        "196.0.5.0/24",
        "196.0.26.0/24",
        "196.0.27.0/24",
        "196.0.29.0/24",
        "196.0.35.0/24",
        "196.6.203.0/24",
        "196.6.215.0/24",
        "196.8.202.0/24",
        "196.8.210.0/24",
        "196.13.255.0/24",
        "196.43.217.0/24",
        "196.43.239.0/24",
        "196.43.246.0/24",
        "196.46.0.0/24",
        "197.249.0.0/24",
        "197.249.1.0/24",
        "197.249.4.0/22",
        "197.249.8.0/21",
        "197.249.240.0/21",
        "196.216.232.0/23",
        "196.223.254.0/24",
        "197.176.0.0/14",
        "197.180.0.0/14",
        "41.80.0.0/16",
        "41.81.0.0/16",
        "41.90.4.0/23",
        "41.90.16.0/20",
        "105.56.0.0/13",
        "197.249.128.0/19",
        "197.249.160.0/19",
        "197.249.192.0/19",
        "41.63.192.0/18",
        "105.232.0.0/17",
        "105.232.128.0/17",
        "41.221.64.0/24",
        "41.221.65.0/24",
        "41.221.66.0/24",
        "41.221.67.0/24",
        "41.221.68.0/24",
        "41.221.69.0/24",
        "41.221.70.0/24",
        "41.221.71.0/24",
        "41.221.72.0/24",
        "41.221.73.0/24",
        "41.221.74.0/24",
        "41.221.75.0/24",
        "41.221.76.0/24",
        "41.221.77.0/24",
        "41.221.78.0/24",
        "41.221.79.0/24",
        "196.22.48.0/24",
        "196.22.49.0/24",
        "196.22.50.0/24",
        "196.22.51.0/24",
        "196.22.52.0/24",
        "196.22.53.0/24",
        "196.22.54.0/24",
        "196.22.55.0/24",
        "196.22.56.0/24",
        "196.22.57.0/24",
        "196.22.58.0/24",
        "196.22.59.0/24",
        "196.22.60.0/24",
        "196.22.61.0/24",
        "154.73.220.0/22",
        "154.117.158.0/24",
        "196.216.242.0/24",
        "196.216.243.0/24",
        "197.248.2.0/24",
        "197.248.4.0/24",
        "197.248.5.0/24",
        "197.248.7.0/24",
        "197.248.8.0/24",
        "197.248.9.0/24",
        "197.248.128.0/24",
        "197.248.129.0/24",
        "31.148.20.0/24",
        "185.59.252.0/22",
        "103.198.184.0/24",
        "103.50.254.0/24",
        "199.245.187.0/24",
        "103.50.254.0/24",
        "195.225.34.0/23",
        "103.50.254.0/24",
        "195.225.34.0/23",
        "96.9.152.0/24",
        "46.149.48.0/23",
        "46.149.52.0/23",
        "46.149.60.0/22",
        "103.50.254.0/24",
        "103.198.185.0/24",
        "103.50.254.0/24",
        "103.198.185.0/24",
        "199.245.187.0/24",
        "202.3.226.0/23",
        "120.50.4.0/23",
        "199.245.187.0/24",
        "197.157.218.0/24",
        "202.3.224.0/19",
        "103.50.254.0/24",
        "46.149.48.0/23",
        "46.149.52.0/23",
        "46.149.60.0/22",
        "185.59.252.0/22",
        "197.157.218.0/24",
        "207.179.89.0/24",
        "103.198.184.0/24",
        "103.198.185.0/24",
        "185.59.252.0/22",
        "103.50.254.0/24",
        "41.74.0.0/24",
        "41.74.0.0/20",
        "41.74.1.0/24",
        "41.74.2.0/24",
        "41.74.3.0/24",
        "41.74.4.0/24",
        "41.74.5.0/24",
        "41.74.6.0/24",
        "41.74.7.0/24",
        "41.74.8.0/23",
        "41.74.8.0/22",
        "41.74.9.0/24",
        "41.74.10.0/23",
        "41.74.12.0/24",
        "41.190.65.0/24",
        "41.190.66.0/24",
        "41.191.84.0/22",
        "41.223.248.0/22",
        "154.127.33.0/24",
        "154.127.34.0/24",
        "154.127.34.0/23",
        "154.127.35.0/24",
        "154.127.32.0/24",
        "154.127.32.0/23",
        "196.46.153.0/24",
        "120.50.4.0/23",
        "199.245.187.0/24",
        "103.50.254.0/24",
        "199.245.187.0/24",
        "203.55.144.0/24",
        "169.239.112.0/22",
        "185.59.252.0/22",
        "208.78.198.0/24",
        "208.86.218.0/24",
        "208.86.219.0/24",
        "208.86.220.0/24",
        "41.70.8.0/21",
        "154.66.122.0/24",
        "154.66.123.0/24",
        "103.50.254.0/24",
        "203.55.144.0/24",
        "203.57.91.0/24",
        "69.89.100.0/23",
        "69.89.110.0/24",
        "69.89.123.0/24",
        "103.50.254.0/24",
        "185.59.252.0/22"
    };

    patricia_trie_t pt;
    patinit(&pt, AF_INET);
    for (uint i = 0; i < countof(prefixes); i++)
        patinsert(&pt, pfx(prefixes[i]), NULL);

    trienode_t *m = patsearchexact(&pt, pfx("124.240.201.0/24"));
    CU_ASSERT(m != NULL);

    patdestroy(&pt);
}

