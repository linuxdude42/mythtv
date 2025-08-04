/*
 *  Class TestHls
 *
 *  Copyright (c) David Hampton 2025
 *
 * See the file LICENSE for licensing information.
 */

#include <QTest>

class TestHls : public QObject
{
    Q_OBJECT

  private slots:
    static void initTestCase();
    static void test_parsem3u8(void);
    static void cleanupTestCase();
};
