BEGIN;

-- CREATE TABLE users (
--     id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
--     name TEXT NOT NULL UNIQUE,
--     email TEXT NOT NULL UNIQUE,
--     password_hash TEXT,
--     is_banned BOOLEAN NOT NULL DEFAULT FALSE,
--     avatar BYTEA
-- );

CREATE TABLE conversations (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT,
    is_group BOOLEAN NOT NULL DEFAULT FALSE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE participants (
    conversation_id UUID NOT NULL,
    user_id UUID NOT NULL,
    joined_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (conversation_id, user_id),
    FOREIGN KEY (conversation_id) REFERENCES conversations(id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE messages (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    sender_id UUID NOT NULL,
    content TEXT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (sender_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE conversation_messages (
    conversation_id UUID NOT NULL,
    message_id UUID NOT NULL,
    PRIMARY KEY (conversation_id, message_id),
    FOREIGN KEY (conversation_id) REFERENCES conversations(id) ON DELETE CASCADE,
    FOREIGN KEY (message_id) REFERENCES messages(id) ON DELETE CASCADE
);

COMMIT;


CREATE INDEX ix_participants_user_id ON participants (user_id);
CREATE INDEX ix_conversation_messages_conversation_id ON conversation_messages (conversation_id);