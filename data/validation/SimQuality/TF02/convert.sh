#!/bin/bash

# Isotrop

python ../../../../scripts/Python/tsvtool.py --output ./results/NANDRAD2_Isotrop_minutely.tsv extract_columns ./SimQuality_TF02/results/loads_minutely.tsv "0|1|2|3|4|12|20|28|5|13|21|29|6|14|22|30|7|15|23|31|8|16|24|32|9|17|25|33|10|18|26|34|11|19|27|35|36|37|45|53|61|38|46|54|62|39|47|55|63|40|48|56|64|41|49|57|65|42|50|58|66|43|51|59|67|44|52|60|68" "Time [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIR_-_0|DIR_N_90|DIR_N_60|DIR_N_45|DIR_N_30|DIR_N-E_90|DIR_N-E_60|DIR_N-E_45|DIR_N-E_30|DIR_E_90|DIR_E_60|DIR_E_45|DIR_E_30|DIR_S-E_90|DIR_S-E_60|DIR_S-E_45|DIR_S-E_30|DIR_S_90|DIR_S_60|DIR_S_45|DIR_S_30|DIR_S-W_90|DIR_S-W_60|DIR_S-W_45|DIR_S-W_30|DIR_W_90|DIR_W_60|DIR_W_45|DIR_W_30|DIR_N-W_90|DIR_N-W_60|DIR_N-W_45|DIR_N-W_30|DIF_-_0|DIF_N_90|DIF_N_60|DIF_N_45|DIF_N_30|DIF_N-E_90|DIF_N-E_60|DIF_N-E_45| DIF_N-E_30|DIF_E_90|DIF_E_60|DIF_E_45|DIF_E_30|DIF_S-E_90|DIF_S-E_60|DIF_S-E_45|DIF_S-E_30|DIF_S_90|DIF_S_60|DIF_S_45|DIF_S_30|DIF_S-W_90|DIF_S-W_60|DIF_S-W_45|DIF_S-W_30|DIF_W_90|DIF_W_60|DIF_W_45|DIF_W_30|DIF_N-W_90|DIF_N-W_60|DIF_N-W_45|DIF_N-W_30"

python ../../../../scripts/Python/tsvtool.py --output ./results/NANDRAD2_Isotrop_hourly.tsv extract_columns ./SimQuality_TF02/results/load_integrals.tsv "0|1|2|3|4|12|20|28|5|13|21|29|6|14|22|30|7|15|23|31|8|16|24|32|9|17|25|33|10|18|26|34|11|19|27|35|36|37|45|53|61|38|46|54|62|39|47|55|63|40|48|56|64|41|49|57|65|42|50|58|66|43|51|59|67|44|52|60|68" "Time [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIR_-_0|DIR_N_90|DIR_N_60|DIR_N_45|DIR_N_30|DIR_N-E_90|DIR_N-E_60|DIR_N-E_45|DIR_N-E_30|DIR_E_90|DIR_E_60|DIR_E_45|DIR_E_30|DIR_S-E_90|DIR_S-E_60|DIR_S-E_45|DIR_S-E_30|DIR_S_90|DIR_S_60|DIR_S_45|DIR_S_30|DIR_S-W_90|DIR_S-W_60|DIR_S-W_45|DIR_S-W_30|DIR_W_90|DIR_W_60|DIR_W_45|DIR_W_30|DIR_N-W_90|DIR_N-W_60|DIR_N-W_45|DIR_N-W_30|DIF_-_0|DIF_N_90|DIF_N_60|DIF_N_45|DIF_N_30|DIF_N-E_90|DIF_N-E_60|DIF_N-E_45| DIF_N-E_30|DIF_E_90|DIF_E_60|DIF_E_45|DIF_E_30|DIF_S-E_90|DIF_S-E_60|DIF_S-E_45|DIF_S-E_30|DIF_S_90|DIF_S_60|DIF_S_45|DIF_S_30|DIF_S-W_90|DIF_S-W_60|DIF_S-W_45|DIF_S-W_30|DIF_W_90|DIF_W_60|DIF_W_45|DIF_W_30|DIF_N-W_90|DIF_N-W_60|DIF_N-W_45|DIF_N-W_30"

# With Perez
python ../../../../scripts/Python/tsvtool.py --output ./results/NANDRAD2_Perez_minutely.tsv extract_columns ./SimQuality_TF02_Perez/results/loads_minutely.tsv "0|1|2|3|4|12|20|28|5|13|21|29|6|14|22|30|7|15|23|31|8|16|24|32|9|17|25|33|10|18|26|34|11|19|27|35|36|37|45|53|61|38|46|54|62|39|47|55|63|40|48|56|64|41|49|57|65|42|50|58|66|43|51|59|67|44|52|60|68" "Time [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIR_-_0|DIR_N_90|DIR_N_60|DIR_N_45|DIR_N_30|DIR_N-E_90|DIR_N-E_60|DIR_N-E_45|DIR_N-E_30|DIR_E_90|DIR_E_60|DIR_E_45|DIR_E_30|DIR_S-E_90|DIR_S-E_60|DIR_S-E_45|DIR_S-E_30|DIR_S_90|DIR_S_60|DIR_S_45|DIR_S_30|DIR_S-W_90|DIR_S-W_60|DIR_S-W_45|DIR_S-W_30|DIR_W_90|DIR_W_60|DIR_W_45|DIR_W_30|DIR_N-W_90|DIR_N-W_60|DIR_N-W_45|DIR_N-W_30|DIF_-_0|DIF_N_90|DIF_N_60|DIF_N_45|DIF_N_30|DIF_N-E_90|DIF_N-E_60|DIF_N-E_45| DIF_N-E_30|DIF_E_90|DIF_E_60|DIF_E_45|DIF_E_30|DIF_S-E_90|DIF_S-E_60|DIF_S-E_45|DIF_S-E_30|DIF_S_90|DIF_S_60|DIF_S_45|DIF_S_30|DIF_S-W_90|DIF_S-W_60|DIF_S-W_45|DIF_S-W_30|DIF_W_90|DIF_W_60|DIF_W_45|DIF_W_30|DIF_N-W_90|DIF_N-W_60|DIF_N-W_45|DIF_N-W_30"

python ../../../../scripts/Python/tsvtool.py --output ./results/NANDRAD2_Perez_hourly.tsv extract_columns ./SimQuality_TF02_Perez/results/load_integrals.tsv "0|1|2|3|4|12|20|28|5|13|21|29|6|14|22|30|7|15|23|31|8|16|24|32|9|17|25|33|10|18|26|34|11|19|27|35|36|37|45|53|61|38|46|54|62|39|47|55|63|40|48|56|64|41|49|57|65|42|50|58|66|43|51|59|67|44|52|60|68" "Time [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIR_-_0|DIR_N_90|DIR_N_60|DIR_N_45|DIR_N_30|DIR_N-E_90|DIR_N-E_60|DIR_N-E_45|DIR_N-E_30|DIR_E_90|DIR_E_60|DIR_E_45|DIR_E_30|DIR_S-E_90|DIR_S-E_60|DIR_S-E_45|DIR_S-E_30|DIR_S_90|DIR_S_60|DIR_S_45|DIR_S_30|DIR_S-W_90|DIR_S-W_60|DIR_S-W_45|DIR_S-W_30|DIR_W_90|DIR_W_60|DIR_W_45|DIR_W_30|DIR_N-W_90|DIR_N-W_60|DIR_N-W_45|DIR_N-W_30|DIF_-_0|DIF_N_90|DIF_N_60|DIF_N_45|DIF_N_30|DIF_N-E_90|DIF_N-E_60|DIF_N-E_45| DIF_N-E_30|DIF_E_90|DIF_E_60|DIF_E_45|DIF_E_30|DIF_S-E_90|DIF_S-E_60|DIF_S-E_45|DIF_S-E_30|DIF_S_90|DIF_S_60|DIF_S_45|DIF_S_30|DIF_S-W_90|DIF_S-W_60|DIF_S-W_45|DIF_S-W_30|DIF_W_90|DIF_W_60|DIF_W_45|DIF_W_30|DIF_N-W_90|DIF_N-W_60|DIF_N-W_45|DIF_N-W_30"
