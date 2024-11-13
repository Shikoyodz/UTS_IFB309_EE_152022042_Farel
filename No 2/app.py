# Import pustaka yang diperlukan
from flask import Flask, jsonify, Response
from flask_cors import CORS
import mysql.connector
import json
from datetime import datetime  # Impor modul datetime untuk memformat timestamp

# Inisialisasi aplikasi Flask
app = Flask(__name__)
CORS(app)  # Mengizinkan akses lintas domain (CORS) untuk pengambilan data dari frontend

# Koneksi ke database MySQL
db = mysql.connector.connect(
    host="localhost",            # Alamat host MySQL (biasanya localhost)
    user="root",                 # Username MySQL 
    password="",                 # Password MySQL 
    database="contoh2_iot"       # Nama database yang digunakan
)

# Route untuk meng-handle permintaan GET ke /data
@app.route('/data', methods=['GET'])
def get_data():
    cursor = db.cursor(dictionary=True)  # Membuat objek cursor untuk eksekusi query dengan hasil berupa dictionary

    # Query untuk mengambil 5 data pertama dari tabel tb_cuaca
    query_data = "SELECT * FROM tb_cuaca LIMIT 3"
    cursor.execute(query_data)  # Eksekusi query
    results = cursor.fetchall()  # Ambil semua hasil query dalam bentuk list of dict

    # Menyusun data sensor dari hasil query
    sensor_data = [
        {
            "id": row["id"],  # ID data sensor
            "suhu": row["suhu"],  # Suhu yang tercatat
            "humid": row["humid"],  # Kelembaban
            "lux": row["lux"],  # Kecerahan (lux)
            "timestamp": row["ts"].strftime('%Y-%m-%d %H:%M:%S') if isinstance(row["ts"], datetime) else row["ts"]  # Format timestamp
        }
        for row in results  # Iterasi setiap baris hasil query untuk menyusun dictionary
    ]

    # Query untuk menghitung suhu maksimum, suhu minimum, dan suhu rata-rata dari tabel tb_cuaca
    query_stats = """
        SELECT 
            MAX(suhu) AS maks_suhu,  # Maksimum suhu
            MIN(suhu) AS min_suhu,  # Minimum suhu
            ROUND(AVG(suhu), 2) AS rata_suhu  # Rata-rata suhu, dibulatkan 2 angka desimal
        FROM tb_cuaca
    """
    cursor.execute(query_stats)  # Eksekusi query statistik suhu
    stats_result = cursor.fetchone()  # Ambil satu hasil query

    # Struktur data statistik suhu
    stats = {
        "suhu_max": stats_result["maks_suhu"],  # Suhu maksimum
        "suhu_min": stats_result["min_suhu"],  # Suhu minimum
        "suhu_rate": stats_result["rata_suhu"]  # Suhu rata-rata
    }

    # Query untuk mengambil bulan dan tahun maksimum dari data yang ada di tabel tb_cuaca
    query_month_year_max = """
        SELECT DISTINCT YEAR(ts) AS year, MONTH(ts) AS month 
        FROM tb_cuaca 
        ORDER BY year DESC, month DESC 
        LIMIT 2  # Ambil dua data bulan dan tahun maksimum
    """
    cursor.execute(query_month_year_max)  # Eksekusi query bulan-tahun maksimum
    month_year_results = cursor.fetchall()  # Ambil hasil query

    # Menyusun data bulan dan tahun maksimum
    month_year_max = [
        {
            "month_year": f"{row['month']}-{row['year']}"  # Gabungkan bulan dan tahun dalam format 'bulan-tahun'
        }
        for row in month_year_results  # Iterasi hasil query bulan-tahun
    ]

    # Gabungkan semua data dalam bentuk JSON terstruktur
    response_data = {
        "suhu_max": stats["suhu_max"],  # Suhu maksimum
        "suhu_min": stats["suhu_min"],  # Suhu minimum
        "suhu_rate": stats["suhu_rate"],  # Suhu rata-rata
        "nilai_suhu_max_humid_max": sensor_data,  # Data suhu, kelembaban, kecerahan, timestamp
        "month_year_max": month_year_max  # Data bulan dan tahun maksimum
    }

    # Mengonversi data Python ke format JSON dan mengirimkan response dengan tipe mimetype 'application/json'
    json_data = json.dumps(response_data, indent=4)  # Mengonversi data ke format JSON dengan indentasi
    return Response(json_data, mimetype='application/json')  # Mengembalikan JSON sebagai response

# Menjalankan aplikasi Flask di mode debug pada port 5000
if __name__ == '__main__':
    app.run(debug=True, port=5000)
