from flask import Flask, jsonify, render_template
from routes.send import send_route
from db import auth_db, db

app = Flask(__name__)
auth_db()

@app.route("/")
def mainpage():
    reqTable = []
    reqTable = db.reference().get()
    del reqTable["UPDATE"]
    reqTable = list(reqTable.values())
    reqUpdate = db.reference("/"+"UPDATE").get()
    return render_template("index.html", day=reqUpdate["data"], hour=reqUpdate["hora"], measure=reqUpdate["medida"], items=reqTable)

@app.errorhandler(404)
def invalid_route(e):
    return "Invalid Route."

app.register_blueprint(send_route, url_prefix="/send")

if __name__ == "__main__":
    app.run(debug=False, port=3000)