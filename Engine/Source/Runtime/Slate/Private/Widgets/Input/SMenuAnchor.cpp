// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
 
#include "SlatePrivatePCH.h"


/**
 * Construct this widget
 *
 * @param	InArgs	The declaration data for this widget
 */
void SMenuAnchor::Construct( const FArguments& InArgs )
{
	Children.Add( new FSimpleSlot() );
	Children.Add( new FSimpleSlot() );
	

	Children[0]
	.Padding(0)
	[
		InArgs._Content.Widget
	];
	MenuContent = InArgs._MenuContent;
	OnGetMenuContent = InArgs._OnGetMenuContent;
	OnMenuOpenChanged = InArgs._OnMenuOpenChanged;
	Placement = InArgs._Placement;
	Method = InArgs._Method;
}


void SMenuAnchor::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	SPanel::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	TSharedPtr<SWindow> PopupWindow = PopupWindowPtr.Pin();
	if ( IsOpenViaCreatedWindow() )
	{
		// Figure out where our attached pop-up window should be placed.
		const FVector2D PopupContentDesiredSize = PopupWindow->GetContent()->GetDesiredSize();
		FGeometry PopupGeometry = ComputeMenuPlacement( AllottedGeometry, PopupContentDesiredSize, Placement.Get( ) );
		const FVector2D NewPosition = PopupGeometry.AbsolutePosition;
		const FVector2D NewSize = PopupGeometry.GetDrawSize( );

		const FSlateRect NewShape = FSlateRect( NewPosition.X, NewPosition.Y, NewPosition.X + NewSize.X, NewPosition.Y + NewSize.Y );

		// We made a window for showing the popup.
		// Update the window's position!
		if( PopupWindow->IsMorphing() )
		{
			if( NewShape != PopupWindow->GetMorphTargetShape() )
			{
				// Update the target shape
				PopupWindow->UpdateMorphTargetShape( NewShape );
				// Set size immediately if not morphing size
				if(!PopupWindow->IsMorphingSize())
				{
					PopupWindow->ReshapeWindow( PopupWindow->GetPositionInScreen(), NewSize );
				}
			}
		}
		else
		{
			const FVector2D WindowPosition = PopupWindow->GetPositionInScreen();
			const FVector2D WindowSize = PopupWindow->GetSizeInScreen();
			if ( NewPosition != WindowPosition || NewSize != WindowSize )
			{
#if PLATFORM_LINUX
				// @FIXME: for some reason, popups reshaped here do not trigger OnWindowMoved() callback,
				// so we manually set cached position to where we expect it to be. Note the order of operations - (before Reshape) - 
				// still giving the callback a chance to change it.
				// This needs to be investigated (tracked as TTP #347674).
				PopupWindow->SetCachedScreenPosition( NewPosition );
#endif // PLATFORM_LINUX
				PopupWindow->ReshapeWindow( NewShape );
			}
		}
	}

	/** The tick is ending, so the window was not dismissed this tick. */
	bDismissedThisTick = false;
}

void SMenuAnchor::OnArrangeChildren( const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren ) const
{
	ArrangeSingleChild( AllottedGeometry, ArrangedChildren, Children[0], FVector2D::UnitVector);
	if ( IsOpen() )
	{
		// @todo umg na AllottedGeometry here is in Window-Space when computed via OnPaint.
		//           The incorrect geometry causes the popup to fail workspace-edge tests.
		const FGeometry PopupGeometry = ComputeMenuPlacement( AllottedGeometry, Children[1].GetWidget()->GetDesiredSize(), Placement.Get() );
		ArrangedChildren.AddWidget( FArrangedWidget( Children[1].GetWidget(), PopupGeometry ) );
	}
}

FVector2D SMenuAnchor::ComputeDesiredSize() const
{
	return Children[0].GetWidget()->GetDesiredSize();
}

FChildren* SMenuAnchor::GetChildren()
{
	return &Children;
}

int32 SMenuAnchor::OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const
{
	FArrangedChildren ArrangedChildren( EVisibility::Visible );
	ArrangeChildren( AllottedGeometry, ArrangedChildren );
	
	// There may be zero elements in this array if our child collapsed/hidden
	if ( ArrangedChildren.Num() > 0 )
	{
		const FArrangedWidget& FirstChild = ArrangedChildren[0];

		// In the case where the user doesn't provide content to the menu anchor, the null widget
		// wont appear in the visible set of arranged children, so only immediately paint the first child,
		// if it's visible and matches the first slot content.
		const bool bHasArrangedAnchorContent = FirstChild.Widget == Children[0].GetWidget();
		if ( bHasArrangedAnchorContent )
		{
			const FSlateRect ChildClippingRect = AllottedGeometry.GetClippingRect().IntersectionWith(MyClippingRect);
			LayerId = FirstChild.Widget->Paint(Args.WithNewParent(this), FirstChild.Geometry, ChildClippingRect, OutDrawElements, LayerId + 1, InWidgetStyle, ShouldBeEnabled(bParentEnabled));
		}

		const bool bIsOpen = IsOpen();

		if ( bIsOpen )
		{
			// In the case where the anchor content is present and visible, it's the 1 index child, in the case
			// where the anchor content is invisible, it's the 0 index child.
			FArrangedWidget* PopupChild = nullptr;
			if ( bHasArrangedAnchorContent && ArrangedChildren.Num() > 1 )
			{
				PopupChild = &ArrangedChildren[1];
			}
			else if ( !bHasArrangedAnchorContent && ArrangedChildren.Num() == 1 )
			{
				PopupChild = &ArrangedChildren[0];
			}

			if ( PopupChild != nullptr )
			{
				 OutDrawElements.QueueDeferredPainting(
					FSlateWindowElementList::FDeferredPaint(PopupChild->Widget, Args, PopupChild->Geometry, MyClippingRect, InWidgetStyle, bParentEnabled));
			}
		} 
	}

	return LayerId;
}

static FVector2D GetMenuOffsetForPlacement(const FGeometry& AllottedGeometry, EMenuPlacement PlacementMode, FVector2D PopupSizeLocalSpace)
{
	switch ( PlacementMode )
	{
	case MenuPlacement_AboveAnchor:
		return FVector2D(0.0f, -AllottedGeometry.GetLocalSize().Y - PopupSizeLocalSpace.Y);
	case MenuPlacement_ComboBoxRight:
		return FVector2D(AllottedGeometry.GetLocalSize().X - PopupSizeLocalSpace.X, 0.0f);
	case MenuPlacement_MenuLeft:
		return FVector2D(-AllottedGeometry.GetLocalSize().X - PopupSizeLocalSpace.X, 0.0f);
	case MenuPlacement_CenteredBelowAnchor:
		return FVector2D(-((PopupSizeLocalSpace.X / 2 ) - (AllottedGeometry.GetLocalSize().X / 2)), 0.0f);
	default:
		return FVector2D::ZeroVector;
	}
}

FGeometry SMenuAnchor::ComputeMenuPlacement( const FGeometry& AllottedGeometry, const FVector2D& PopupDesiredSize, EMenuPlacement PlacementMode )
{
	// Compute the popup size, offset, and anchor rect  in local space
	const FVector2D PopupSizeLocalSpace = (PlacementMode == MenuPlacement_ComboBox || PlacementMode == MenuPlacement_ComboBoxRight)
		? FVector2D( FMath::Max( AllottedGeometry.Size.X, PopupDesiredSize.X ), PopupDesiredSize.Y )
		: PopupDesiredSize;
	const FVector2D OffsetLocalSpace = GetMenuOffsetForPlacement(AllottedGeometry, PlacementMode, PopupSizeLocalSpace);
	const FSlateRect AnchorLocalSpace = FSlateRect::FromPointAndExtent(OffsetLocalSpace, AllottedGeometry.Size);
	const EOrientation Orientation = (PlacementMode == MenuPlacement_MenuRight || PlacementMode == MenuPlacement_MenuLeft) ? Orient_Horizontal : Orient_Vertical;
	
	// ask the application to compute the proper desktop offset for the anchor. This requires the offsets to be in desktop space.
	const FVector2D NewPositionDesktopSpace = FSlateApplication::Get().CalculatePopupWindowPosition( 
		TransformRect(AllottedGeometry.GetAccumulatedLayoutTransform(), AnchorLocalSpace), 
		TransformVector(AllottedGeometry.GetAccumulatedLayoutTransform(), PopupSizeLocalSpace), 
		Orientation );

	// transform the desktop offset into local space and use that as the layout transform for the child content.
	return AllottedGeometry.MakeChild(
		PopupSizeLocalSpace,
		FSlateLayoutTransform(TransformPoint(Inverse(AllottedGeometry.GetAccumulatedLayoutTransform()), NewPositionDesktopSpace)));
}

void SMenuAnchor::RequestClosePopupWindow( const TSharedRef<SWindow>& PopupWindow )
{
	bDismissedThisTick = true;
	if (ensure(IsOpenViaCreatedWindow()))
	{
		FSlateApplication::Get().RequestDestroyWindow(PopupWindow);
		
		if (OnMenuOpenChanged.IsBound())
		{
			OnMenuOpenChanged.Execute(false);
		}
	}
}

void SMenuAnchor::OnClickedOutsidePopup()
{
	bDismissedThisTick = true;
	FSlateApplication::Get().GetPopupSupport().UnregisterClickNotification( FOnClickedOutside::CreateSP(this, &SMenuAnchor::OnClickedOutsidePopup) );
	SetIsOpen(false);	
}

bool SMenuAnchor::IsOpenAndReusingWindow() const
{
	return Children[1].GetWidget() != SNullWidget::NullWidget;
}

bool SMenuAnchor::IsOpenViaCreatedWindow() const
{
	return PopupWindowPtr.Pin().IsValid() && Children[1].GetWidget() == SNullWidget::NullWidget;
}

void SMenuAnchor::SetContent(TSharedRef<SWidget> InContent)
{
	Children[0]
	.Padding(0)
	[
		InContent
	];
}

void SMenuAnchor::SetMenuContent(TSharedRef<SWidget> InMenuContent)
{
	MenuContent = InMenuContent;
}

EPopupMethod QueryPopupMethod(const FWidgetPath& PathToQuery)
{
	for (int32 WidgetIndex = PathToQuery.Widgets.Num() - 1; WidgetIndex >= 0; --WidgetIndex)
	{
		TOptional<EPopupMethod> PopupMethod = PathToQuery.Widgets[WidgetIndex].Widget->OnQueryPopupMethod();
		if (PopupMethod.IsSet())
		{
			return PopupMethod.GetValue();
		}
	}

	return EPopupMethod::CreateNewWindow;
}


/**
 * Open or close the popup
 *
 * @param InIsOpen  If true, open the popup. Otherwise close it.
 */
void SMenuAnchor::SetIsOpen( bool InIsOpen, const bool bFocusMenu )
{
	// Prevent redundant opens/closes
	if ( IsOpen() != InIsOpen )
	{
		if ( InIsOpen )
		{
			TSharedPtr< SWidget > MenuContentPtr = OnGetMenuContent.IsBound() ? OnGetMenuContent.Execute() : MenuContent;

			if ( MenuContentPtr.IsValid() )
			{
				// OPEN POPUP
				if ( OnMenuOpenChanged.IsBound() )
				{
					OnMenuOpenChanged.Execute(true);
				}

				// This can be called at any time so we use the push menu override that explicitly allows us to specify our parent

				// Figure out where the menu anchor is on the screen, so we can set the initial position of our pop-up window
				SlatePrepass();

				// NOTE: Careful, GeneratePathToWidget can be reentrant in that it can call visibility delegates and such
				FWidgetPath MyWidgetPath;
				FSlateApplication::Get().GeneratePathToWidgetUnchecked(AsShared(), MyWidgetPath);
				if (ensure(MyWidgetPath.IsValid()))
				{
					const FGeometry& MyGeometry = MyWidgetPath.Widgets.Last().Geometry;

					// @todo Slate: This code does not properly propagate the layout scale of the widget we are creating the popup for.
					// The popup instead has a scale of one, but a computed size as if the contents were scaled. This code should ideally
					// wrap the contents with an SFxWidget that applies the necessary layout scale. This is a very rare case right now.

					// Figure out how big the content widget is so we can set the window's initial size properly
					TSharedRef< SWidget > MenuContentRef = MenuContentPtr.ToSharedRef();
					MenuContentRef->SlatePrepass();

					// Combo-boxes never size down smaller than the widget that spawned them, but all
					// other pop-up menus are currently auto-sized
					const FVector2D DesiredContentSize = MenuContentRef->GetDesiredSize();  // @todo: This is ignoring any window border size!
					const EMenuPlacement PlacementMode = Placement.Get();

					const FVector2D NewPosition = MyGeometry.AbsolutePosition;
					FVector2D NewWindowSize = DesiredContentSize;
					const FVector2D SummonLocationSize = MyGeometry.Size;

					FPopupTransitionEffect TransitionEffect( FPopupTransitionEffect::None );
					if ( PlacementMode == MenuPlacement_ComboBox || PlacementMode == MenuPlacement_ComboBoxRight )
					{
						TransitionEffect = FPopupTransitionEffect( FPopupTransitionEffect::ComboButton );
						NewWindowSize = FVector2D( FMath::Max( MyGeometry.Size.X, DesiredContentSize.X ), DesiredContentSize.Y );
					}
					else if ( PlacementMode == MenuPlacement_BelowAnchor )
					{
						TransitionEffect = FPopupTransitionEffect( FPopupTransitionEffect::TopMenu );
					}
					else if ( PlacementMode == MenuPlacement_MenuRight )
					{
						TransitionEffect = FPopupTransitionEffect( FPopupTransitionEffect::SubMenu );
					}

					const EPopupMethod PopupMethod = Method.IsSet()
						? Method.GetValue()
						: QueryPopupMethod(MyWidgetPath);

					if (PopupMethod == EPopupMethod::CreateNewWindow)
					{
						// Open the pop-up
						TSharedRef<SWindow> PopupWindow = FSlateApplication::Get().PushMenu( AsShared(), MenuContentRef, NewPosition, TransitionEffect, bFocusMenu, false, NewWindowSize, SummonLocationSize );
						PopupWindow->SetRequestDestroyWindowOverride( FRequestDestroyWindowOverride::CreateSP( this, &SMenuAnchor::RequestClosePopupWindow ) );
						PopupWindowPtr = PopupWindow;
					}
					else
					{
						// We are re-using the current window instead of creating a new one.
						// The popup will be presented via an overlay service.
						ensure(PopupMethod == EPopupMethod::UseCurrentWindow);
						Children[1]
						[
							MenuContentRef
						];

						// We want to support dismissing the popup widget when the user clicks outside it.
						FSlateApplication::Get().GetPopupSupport().RegisterClickNotification( MenuContentRef, FOnClickedOutside::CreateSP( this, &SMenuAnchor::OnClickedOutsidePopup ) );
					}

					if ( bFocusMenu )
					{
						FSlateApplication::Get().SetKeyboardFocus( MenuContentRef, EFocusCause::SetDirectly );
					}
				}
			}
		}
		else
		{
			// CLOSE POPUP

			if (IsOpenAndReusingWindow())
			{
				// Window reuse case; remove child widget.
				Children[1].DetachWidget();
			}
			else
			{
				// New window case
				ensure(IsOpenViaCreatedWindow());
				TSharedPtr<SWindow> PopupWindow = PopupWindowPtr.Pin();
				if (PopupWindow.IsValid())
				{
					// Request that the popup be closed.
					PopupWindow->RequestDestroyWindow();
				}
			}

			if (OnMenuOpenChanged.IsBound())
			{
				OnMenuOpenChanged.Execute(false);
			}
		}
	}
}

bool SMenuAnchor::IsOpen() const
{
	return PopupWindowPtr.Pin().IsValid() || IsOpenAndReusingWindow();
}

bool SMenuAnchor::ShouldOpenDueToClick() const
{
	return !IsOpen() && !bDismissedThisTick;
}

FVector2D SMenuAnchor::GetMenuPosition() const
{
	FVector2D Pos(0,0);
	if( PopupWindowPtr.IsValid() )
	{
		Pos = PopupWindowPtr.Pin()->GetPositionInScreen();
	}
	
	return Pos;
}

bool SMenuAnchor::HasOpenSubMenus() const
{
	bool Result = false;
	if( PopupWindowPtr.IsValid() )
	{
		Result = FSlateApplication::Get().HasOpenSubMenus( PopupWindowPtr.Pin().ToSharedRef() );
	}
	return Result;
}

SMenuAnchor::SMenuAnchor()
	: MenuContent( SNullWidget::NullWidget )
	, bDismissedThisTick( false )
	, Method(CreateNewWindow)
{
}

SMenuAnchor::~SMenuAnchor()
{
	TSharedPtr<SWindow> PopupWindow = PopupWindowPtr.Pin();
	if (PopupWindow.IsValid())
	{
		// Close the Popup window.
		if (IsOpenViaCreatedWindow())
		{
			// Request that the popup be closed.
			PopupWindow->RequestDestroyWindow();
		}
		
		// We no longer have a popup open, so reset all the tracking state associated.
		PopupWindowPtr.Reset();
	}
}
