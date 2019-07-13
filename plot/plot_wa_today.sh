#!/bin/sh

jahr=$(date --date="today" "+%Y")

verz=/auswertung/${jahr}

[ -d ${verz} ] || mkdir -p ${verz}

dateiname=$(date --date="today" "+%Y-%m-%d")

for ext in WAAGE00 WAAGE01 WAAGE02 WAAGE03 WAAGE04 WAAGE05 WAAGE06 WAAGE07
do
	cat /daten/${dateiname}_${ext} | awk '{print $1 "\t" $2 "\t" $3}' > /tmp/${dateiname}_${ext}
done

for ext in BME00
do
	cat /daten/${dateiname}_${ext}  | awk '{print $1 "\t" $2 "\t" $3 "\t" $4}' > /tmp/${dateiname}_${ext}
done

heute=${verz}/heute.$(date --date="today" "+%Y-%m-%d")
datei=${verz}/$(date --date="today" "+%Y-%m-%d")

join /tmp/${dateiname}_BME00 /tmp/${dateiname}_WAAGE00 > /tmp/join1
join /tmp/join1 /tmp/${dateiname}_WAAGE01 > ${datei}.wa0001

join /tmp/${dateiname}_BME00 /tmp/${dateiname}_WAAGE02 > /tmp/join1
join /tmp/join1 /tmp/${dateiname}_WAAGE03 > ${datei}.wa0203

join /tmp/${dateiname}_BME00 /tmp/${dateiname}_WAAGE04 > /tmp/join1
join /tmp/join1 /tmp/${dateiname}_WAAGE05 > ${datei}.wa0405

join /tmp/${dateiname}_BME00 /tmp/${dateiname}_WAAGE06 > /tmp/join1
join /tmp/join1 /tmp/${dateiname}_WAAGE07 > ${datei}.wa0607

echo > ${heute}.wa0001
echo > ${heute}.wa0203
echo > ${heute}.wa0405
echo > ${heute}.wa0607

cat ${datei}.wa0001 | grep -v '^#' >> ${heute}.wa0001
cat ${datei}.wa0203 | grep -v '^#' >> ${heute}.wa0203
cat ${datei}.wa0405 | grep -v '^#' >> ${heute}.wa0405
cat ${datei}.wa0607 | grep -v '^#' >> ${heute}.wa0607

gnuplot -e "datei='${heute}'" -e "ext='wa0001'" -e "stocka='Stock-00'" -e "stockb='Stock-01'" /opt/bienen/bin/waage_daten.plt
gnuplot -e "datei='${heute}'" -e "ext='wa0203'" -e "stocka='Stock-02'" -e "stockb='Stock-03'" /opt/bienen/bin/waage_daten.plt
gnuplot -e "datei='${heute}'" -e "ext='wa0405'" -e "stocka='Stock-04'" -e "stockb='Stock-05'" /opt/bienen/bin/waage_daten.plt
gnuplot -e "datei='${heute}'" -e "ext='wa0607'" -e "stocka='Stock-06'" -e "stockb='Stock-07'" /opt/bienen/bin/waage_daten.plt

mutt -s "Bienenwaage - Schwarm-Alarm $(date --date='today' '+%Y-%m-%d')" -a ${heute}.wa0001.png -a ${heute}.wa0203.png -a ${heute}.wa0405.png -a ${heute}.wa0607.png -- ak@it-klinger.de <<EOF
Bienenwaage - Schwarm-Alarm
$1
${heute}.wa0001.png
${heute}.wa0203.png
${heute}.wa0405.png
${heute}.wa0607.png
EOF

exit 0;

