from firebase_admin import credentials, db, initialize_app, firestore
def auth_db():
    cred = credentials.Certificate("credentials.json")
    initialize_app(cred, {"databaseURL": cred.DATAURL})
    return firestore.client()
