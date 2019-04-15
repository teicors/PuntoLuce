import web
import json
import sys
import MySQLdb as mdb
import logging
import subprocess
import urllib2
#import requests

from threading import Timer
from time import sleep


class RepeatedTimer(object):
    def __init__(self, interval, function, *args, **kwargs):
        self._timer     = None
        self.function   = function
        self.interval   = interval
        self.args       = args
        self.kwargs     = kwargs
        self.is_running = False
        self.start()

    def _run(self):
        self.is_running = False
        self.start()
        self.function(*self.args, **self.kwargs)

    def start(self):
        if not self.is_running:
            self._timer = Timer(self.interval, self._run)
            self._timer.start()
            self.is_running = True

    def stop(self):
        self._timer.cancel()
        self.is_running = False


urls = (
    '/', 'index',
    '/json', 'data_per_post'
)

class data_per_post:
    def GET(self):
        return "Hello, world! data"

    def POST(self):
        logger = logging.getLogger("process-POST")
        try:
            data = json.loads(web.data())
            print data
#            json.dumps(data)
##            data = urlparse.parse_qs(r)
#            print json.loads(r['payload'][0])
#            json_value = json.loads(data)
            print len(data)
            stato=data["stato"]
            basetta=data["ip"]
            pulsante=data["pulsante"]
            valore=data["valore"]
            evento=data["evento"]
            
            
#            if len(data)==3:
#                 led=data["sensorid"]
#                count=data["count"]
#                basetta=data["basetta"]
#            else:
#                basetta=data["basetta"]
#                led=data["sensorid"]
            con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione'); 
            cur = con.cursor(mdb.cursors.DictCursor)
            presence=0
            if evento == 9999:
                presence=1
                print("Received presence")
                sql="select * from illuminazione.impianto where matricola='%s';" % (basetta)
                cur.execute(sql)
                if cur.rowcount==0:
#                    parts = basetta.split('.')
#                    matricola=(int(parts[2])*256+int(parts[3]))
                    sql="insert into illuminazione.impianto (matricola,critica,attivo) values ('%s',0,0);" % (basetta)
                    cur.execute(sql)
                    con.commit()
                sql="update illuminazione.impianto set attivo=1 where matricola='%s';" % (basetta)
                cur.execute(sql)
                con.commit()
                print("Presence of %s" % (basetta))
#                sql="select p.elemento from elemento p, impianto i \
#                    where time(now()) between p.ora_inizio and p.ora_fine \
#                    and p.elemento=i.elemento \
#                    and m.ip='%s' \
#                    and i.matricola=m.matricola \
#                    and m.attivo=1;" % (basetta)
            else:
                print "i am in show"
                print "Valori Led %s Basetta %s Count %s" % (pulsante, basetta, valore)
                logger.debug("Light management")
                sql="select p.elemento from elemento p, impianto i \
                    where time(now()) between p.ora_inizio and p.ora_fine \
                    and p.elemento=i.elemento \
                    and i.matricola='%s' \
                    and i.pulsante=%s \
                    and i.attivo=1;" % (basetta, pulsante)
            with con:
                cur = con.cursor(mdb.cursors.DictCursor)
                cur.execute(sql)
                if cur.rowcount>0:
                    rows = cur.fetchall()
                    for row in rows:
                        elemento=row['elemento']
                        if (stato==2):
                            sql="update illuminazione.elemento set valore=abs(valore-1) where elemento=%s\
                                and time(now()) between ora_inizio and ora_fine " % (elemento)
                            cur.execute(sql)
                            con.commit()

                        sql="select b.matricola, b.pulsante, a.valore from illuminazione.elemento a, illuminazione.impianto b \
                            where time(now()) between ora_inizio and ora_fine \
                        and b.elemento='%s' \
                        and b.elemento=a.elemento \
                        and a.attivo=1;" % (elemento)

                        cur.execute(sql)
                        if cur.rowcount>0:
                            rows = cur.fetchall()
                            for row in rows:
                                ip=row["matricola"]
                                led=row["pulsante"]
                                status=row["valore"]
                                logger.debug("Check Light management IP %s Status %s Led %s " % (ip,status,led))
                                req = 'http://%s/api/output?valore=%s&led=%s' % (ip,status,led)
                                urllib2.urlopen(req).read()
                                print "Valori Led %s Basetta %s Valore %s" % (led, ip, status)
                else:
                    logger.debug("Received various data")
            con.close()
            return
        except IOError as e:
            print "I/O error({0}): {1}".format(e.errno, e.strerror)
        except ValueError:
            print "Could not convert data to an integer."
        except:
            print "Unexpected error:", sys.exc_info()[0]
            raise

#        except Error(e):
#            print e

class index:
    def GET(self):
        return "Hello, world!"
    
def send_conf(text):
    con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione');
    cur = con.cursor(mdb.cursors.DictCursor)
    logger.debug("Send configuration")
    sql="select a.matricola, b.pulsante, a.valore from illuminazione.elemento a, illuminazione.impianto b \
         where time(now()) between ora_inizio and ora_fine \
    and a.elemento=b.elemento \
    and a.attivo=1;"
    cur.execute(sql)
    if cur.rowcount>0:
        rows = cur.fetchall()
        for row in rows:
            ip=row["matricola"]
            led=row["pulsante"]
            status=row["valore"]
            logger.debug("Check send_conf IP %s Status %s Led %s " % (ip,valore,led))
            req = 'http://%s/api/output?status=%s&led=%s' % (ip,status,led)
            urllib2.urlopen(req).read()
    con.close()

def presence(text):
    con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione');
    cur = con.cursor(mdb.cursors.DictCursor)
    logger.debug("Check presence")
    sql="SELECT distinct matricola FROM illuminazione.impianto where attivo=1;"
    cur.execute(sql)
    if cur.rowcount>0:
        rows = cur.fetchall()
        for row in rows:
            ip=row['matricola']
            logger.debug("Check IP %s" % (ip))
            try:
                if subprocess.call("ping -c 1 %s" % ip , shell=True) == 0:
                    print ("host appears to be up")
                else:
                    print ("host appears to be down")
                    con1 = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione');
                    cur1 = con1.cursor(mdb.cursors.DictCursor)
                    sql="update illuminazione.matricole set attivo=0 where matricola='%s';" % (ip)
                    cur1.execute(sql)
                    con1.commit()  
                    con1.close()
            except:
                print ("Ping Error:")
    con.close()


class MyApplication(web.application):
    def run(self, port=5008, *middleware):
        func = self.wsgifunc(*middleware)
        return web.httpserver.runsimple(func, ('10.42.0.1', port))


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    logger = logging.getLogger("server")
    logger.info("starting...")

    rt = RepeatedTimer(600, send_conf, "Send conf") # it auto-starts, no need of rt.start()
    rt = RepeatedTimer(600, presence, "Presence") # it auto-starts, no need of rt.start()

    try:
        app = MyApplication(urls, globals())
        app.run()

    finally:
        rt.stop() # better in a try/finally block to make sure the program ends!
        logger.info("Shutting down")
        if con:    
            con.close()
           
    logger.info("All done")
