#include "GameManager.h"

namespace battleship
{
	GameManager::GameManager()
	{
	}


	GameManager::~GameManager()
	{
	}

	bool GameManager::isPlayerShipsLeft(BattleBoard* board, PlayerEnum player)
	{
		int shipsCount = (player == PlayerEnum::A) ? board->getPlayerAShipCount() : board->getPlayerBShipCount;
		return (shipsCount > 0);
	}

	bool GameManager::isGameOver(BattleBoard* board, bool isPlayerAForfeit, bool isPlayerBForfeit)
	{
		if ((isPlayerAForfeit && isPlayerBForfeit) ||
			(!isPlayerShipsLeft(board, PlayerEnum::A)) ||
			(!isPlayerShipsLeft(board, PlayerEnum::B)))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	IBattleshipGameAlgo* GameManager::switchPlayerTurns(IBattleshipGameAlgo& playerA, IBattleshipGameAlgo& playerB,
														IBattleshipGameAlgo* currPlayer,
														bool isPlayerAForfeit, bool isPlayerBForfeit)
	{
		if ((currPlayer == &playerA) && (!isPlayerBForfeit))
			return &playerB;
		else if (!isPlayerAForfeit)
			return &playerA;
		else
			return &playerB;
	}

	void GameManager::startGame(shared_ptr<BattleBoard> board,
								IBattleshipGameAlgo& playerA, IBattleshipGameAlgo& playerB,
								IGameVisual& visualizer)
	{
		playerA.setBoard(board->getBoardMatrix(), BOARD_SIZE, BOARD_SIZE);
		playerB.setBoard(board->getBoardMatrix(), BOARD_SIZE, BOARD_SIZE);

		IBattleshipGameAlgo* currentPlayer = &playerA;
		bool isPlayerAForfeit = false;
		bool isPlayerBForfeit = false;

		while (!isGameOver(board.get(), isPlayerAForfeit, isPlayerBForfeit))
		{
			// Attack
			auto target = currentPlayer->attack();

			if (target == FORFEIT)
			{	// Player chose not to attack - from now on this player forfeits the game
				if (currentPlayer == &playerA)
					isPlayerAForfeit = true;
				else
					isPlayerBForfeit = true;

				currentPlayer = switchPlayerTurns(playerA, playerB, currentPlayer,
												  isPlayerAForfeit, isPlayerBForfeit);
				continue;
			}

			// Normalize coordinates to 0~BOARD_SIZE-1
			target.first--;
			target.second--;

			// Execute attack move on the board itself and update the game-pieces status
			// We get in return an object that describes the result of the attack
			auto attackedGamePiece = board->executeAttack(target);

			// Notify on attack results
			int attackingPlayerNumber = (currentPlayer == &playerB); // A - 0, B - 1
			AttackResult attackResult;

			if (attackedGamePiece == NULL)
			{	// Miss
				attackResult = AttackResult::Miss;
				currentPlayer = switchPlayerTurns(playerA, playerB, currentPlayer,
												  isPlayerAForfeit, isPlayerBForfeit);
			}
			else if (attackedGamePiece->_lifeLeft == 0)
			{	// Sink
				attackResult = AttackResult::Sink;
			}
			else
			{	// Hit
				attackResult = AttackResult::Hit;
			}

			playerA.notifyOnAttackResult(attackingPlayerNumber, target.first, target.second, attackResult);
			playerB.notifyOnAttackResult(attackingPlayerNumber, target.first, target.second, attackResult);
			visualizer.visualizeAttackResults(target.first, target.second, attackResult);
		}

		visualizer.visualizeEndGame(board, isPlayerAForfeit, isPlayerBForfeit);
	}
}