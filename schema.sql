BEGIN;

CREATE TABLE "users" (
    "id"            SERIAL PRIMARY KEY,
    "name"          TEXT NOT NULL UNIQUE,
    "email"         TEXT NOT NULL UNIQUE,
    "password_hash" TEXT,
    "is_banned"     INTEGER NOT NULL DEFAULT 0,
    "avatar"        BYTEA,
    CHECK ("is_banned" IN (0, 1))
);

CREATE TABLE "messages" (
    "id"            SERIAL PRIMARY KEY,
    "author_id"     INTEGER,
    "text"          TEXT NOT NULL,
    "created_at"    INTEGER NOT NULL,
    FOREIGN KEY("author_id") REFERENCES "users"("id") ON DELETE SET NULL
);

CREATE TABLE "contacts" (
    "id"	        SERIAL PRIMARY KEY,
    "owner_id"	    INTEGER NOT NULL,
    "friend_id"	    INTEGER NOT NULL,
    "name"	        TEXT NOT NULL,
    FOREIGN KEY("owner_id") REFERENCES "users"("id") ON DELETE CASCADE,
    FOREIGN KEY("friend_id") REFERENCES "users"("id") ON DELETE SET NULL
);

CREATE TABLE "contact_messages" (
    "contact_id"	INTEGER,
    "message_id"	INTEGER,
    PRIMARY KEY("contact_id", "message_id"),
    FOREIGN KEY("contact_id") REFERENCES "contacts"("id") ON DELETE CASCADE,
    FOREIGN KEY("message_id") REFERENCES "messages"("id") ON DELETE CASCADE
);

COMMIT;

CREATE INDEX "ix_contacts_owner_id" ON "contacts" (
	"owner_id"
);