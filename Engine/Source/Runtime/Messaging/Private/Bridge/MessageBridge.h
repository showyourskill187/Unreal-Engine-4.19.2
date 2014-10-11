// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once


/**
 * Implements a message bridge.
 */
class FMessageBridge
	: public TSharedFromThis<FMessageBridge, ESPMode::ThreadSafe>
	, public IMessageBridge
	, public IReceiveMessages
	, public ISendMessages
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InAddress The address for this bridge.
	 * @param InBus The message bus that this node is connected to.
	 * @param InSerializer The message serializer to use.
	 * @param InTransport The transport mechanism to use.
	 */
	FMessageBridge( const FMessageAddress InAddress, const IMessageBusRef& InBus, const ISerializeMessagesRef& InSerializer, const ITransportMessagesRef& InTransport );

	/** Destructor. */
	~FMessageBridge();

public:

	// IMessageBridge interface

	virtual void Disable() override;
	virtual void Enable() override;

	virtual bool IsEnabled() const override
	{
		return Enabled;
	}

public:

	// IReceiveMessages interface

	virtual FName GetDebugName() const override
	{
		return *FString::Printf(TEXT("FMessageBridge (%s)"), *Transport->GetDebugName().ToString());
	}

	virtual const FGuid& GetRecipientId() const override
	{
		return Id;
	}

	virtual ENamedThreads::Type GetRecipientThread() const override
	{
		return ENamedThreads::AnyThread;
	}

	virtual bool IsLocal() const override
	{
		return false;
	}

	virtual void ReceiveMessage( const IMessageContextRef& Context ) override;

public:

	// ISendMessages interface

	virtual FMessageAddress GetSenderAddress() override
	{
		return Address;
	}

	virtual void NotifyMessageError( const IMessageContextRef& Context, const FString& Error ) override;

protected:

	/** Shuts down the bridge. */
	void Shutdown();

private:

	/** Callback for message bus shut downs. */
	void HandleMessageBusShutdown();

	/** Callback for received transport messages. */
	void HandleTransportMessageReceived( FArchive& MessageData, const IMessageAttachmentPtr& Attachment, const FGuid& NodeId );

	/** Callback for lost transport endpoints. */
	void HandleTransportNodeLost( const FGuid& LostNodeId );

private:

	/** Holds the bridge's address. */
	FMessageAddress Address;

	/** Holds the address book. */
	FMessageAddressBook AddressBook;

	/** Holds a reference to the bus that this bridge is attached to. */
	IMessageBusPtr Bus;

	/** Hold a flag indicating whether this endpoint is active. */
	bool Enabled;

	/** Holds the bridge's unique identifier (for debugging purposes). */
	const FGuid Id;

	/** Holds the message subscription for outbound messages. */
	IMessageSubscriptionPtr MessageSubscription;

	/** Holds the message serializer. */
	ISerializeMessagesPtr Serializer;

	/** Holds the message transport object. */
	ITransportMessagesPtr Transport;
};
